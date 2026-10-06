#include "NativeExtensions.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

extern "C" DWORD XexLoadImage(LPCSTR szXexName,DWORD dwModuleTypeFlags,DWORD dwMinimumVersion,PHANDLE pHandle);

namespace sd360x {

static std::string baseName(const std::string& p){
 size_t s=p.find_last_of("\\/");
 return s==std::string::npos?p:p.substr(s+1);
}

void PluginManager::scan(){
 items_.clear();
 CreateDirectoryA("Hdd1:\\SeriesDash360",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\plugins",0);
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA("Hdd1:\\SeriesDash360\\plugins\\*.xex",&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  PluginInfo p;
  p.name=fd.cFileName;
  p.path=std::string("Hdd1:\\SeriesDash360\\plugins\\")+fd.cFileName;
  items_.push_back(p);
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

bool PluginManager::load(size_t i){
 if(i>=items_.size())return false;
 HANDLE module=0;
 DWORD r=XexLoadImage(items_[i].path.c_str(),8,0,&module);
 items_[i].loaded=(r==0);
 return items_[i].loaded;
}

void PluginManager::loadAll(){
 scan();
 for(size_t i=0;i<items_.size();++i)load(i);
}

static std::string trim(const std::string& s){
 size_t a=s.find_first_not_of(" \t\r\n");
 if(a==std::string::npos)return "";
 size_t b=s.find_last_not_of(" \t\r\n");
 return s.substr(a,b-a+1);
}

bool ScriptRuntime::execute(const std::string& raw,NativeRuntime& runtime,StateStore& state){
 std::string line=trim(raw);
 if(line.empty()||line[0]=='#'||line[0]==';')return true;
 size_t sp=line.find(' ');
 std::string cmd=sp==std::string::npos?line:line.substr(0,sp);
 std::string arg=sp==std::string::npos?"":trim(line.substr(sp+1));

 if(cmd=="scan"){
  runtime.scan();
  state.apply(runtime.titles());
  return true;
 }
 if(cmd=="theme"){
  state.setTheme(arg);
  state.save();
  return true;
 }
 if(cmd=="favorite"){
  DWORD id=(DWORD)strtoul(arg.c_str(),0,16);
  state.toggleFavorite(id);
  state.apply(runtime.titles());
  return true;
 }
 if(cmd=="cover"){
  DWORD id=(DWORD)strtoul(arg.c_str(),0,16);
  for(size_t i=0;i<runtime.titles().size();++i){
   if(runtime.titles()[i].titleId==id)return runtime.refreshCover(i);
  }
  return false;
 }
 if(cmd=="launch"){
  DWORD id=(DWORD)strtoul(arg.c_str(),0,16);
  for(size_t i=0;i<runtime.titles().size();++i){
   if(runtime.titles()[i].titleId==id){
    state.recordPlayed(id);
    return runtime.launch(i);
   }
  }
  return false;
 }
 if(cmd=="category"){
  size_t p=arg.find(' ');
  if(p==std::string::npos)return false;
  DWORD id=(DWORD)strtoul(arg.substr(0,p).c_str(),0,16);
  state.setCategory(id,trim(arg.substr(p+1)));
  state.apply(runtime.titles());
  return true;
 }
 if(cmd=="title"){
  size_t p=arg.find(' ');
  if(p==std::string::npos)return false;
  DWORD id=(DWORD)strtoul(arg.substr(0,p).c_str(),0,16);
  state.setCustomTitle(id,trim(arg.substr(p+1)));
  state.apply(runtime.titles());
  return true;
 }
 return false;
}

bool ScriptRuntime::runFile(const std::string& path,NativeRuntime& runtime,StateStore& state){
 FILE* f=fopen(path.c_str(),"rb");
 if(!f)return false;
 char line[1024];
 bool ok=true;
 while(fgets(line,sizeof(line),f)){
  if(!execute(line,runtime,state))ok=false;
 }
 fclose(f);
 return ok;
}

void ScriptRuntime::runStartupScripts(NativeRuntime& runtime,StateStore& state){
 CreateDirectoryA("Hdd1:\\SeriesDash360\\scripts",0);
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA("Hdd1:\\SeriesDash360\\scripts\\*.sd360",&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  runFile(std::string("Hdd1:\\SeriesDash360\\scripts\\")+fd.cFileName,runtime,state);
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

SystemLinkService::SystemLinkService():socket_(INVALID_SOCKET),port_(30720),lastBeacon_(0){}
SystemLinkService::~SystemLinkService(){stop();}

bool SystemLinkService::start(unsigned short port){
 port_=port;
 socket_=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
 if(socket_==INVALID_SOCKET)return false;
 BOOL yes=TRUE;
 setsockopt(socket_,SOL_SOCKET,SO_BROADCAST,(const char*)&yes,sizeof(yes));
 u_long nb=1;
 ioctlsocket(socket_,FIONBIO,&nb);
 sockaddr_in a;
 ZeroMemory(&a,sizeof(a));
 a.sin_family=AF_INET;
 a.sin_addr.s_addr=INADDR_ANY;
 a.sin_port=htons(port_);
 if(bind(socket_,(sockaddr*)&a,sizeof(a))==SOCKET_ERROR){
  closesocket(socket_);
  socket_=INVALID_SOCKET;
  return false;
 }
 return true;
}

void SystemLinkService::sendBeacon(DWORD titleId,const std::string& title){
 if(socket_==INVALID_SOCKET)return;
 char packet[256];
 sprintf(packet,"SD360|%08X|%s",(unsigned)titleId,title.c_str());
 sockaddr_in to;
 ZeroMemory(&to,sizeof(to));
 to.sin_family=AF_INET;
 to.sin_addr.s_addr=INADDR_BROADCAST;
 to.sin_port=htons(port_);
 sendto(socket_,packet,strlen(packet),0,(sockaddr*)&to,sizeof(to));
}

void SystemLinkService::receive(){
 if(socket_==INVALID_SOCKET)return;
 for(;;){
  char b[512];
  sockaddr_in from;
  int flen=sizeof(from);
  int n=recvfrom(socket_,b,sizeof(b)-1,0,(sockaddr*)&from,&flen);
  if(n<=0)break;
  b[n]=0;
  if(strncmp(b,"SD360|",6))continue;
  char* p1=strchr(b+6,'|');
  if(!p1)continue;
  *p1=0;
  DWORD id=(DWORD)strtoul(b+6,0,16);
  const char* name=p1+1;
  char ip[64];
  sprintf(ip,"%u.%u.%u.%u",
   (unsigned)((ntohl(from.sin_addr.s_addr)>>24)&255),
   (unsigned)((ntohl(from.sin_addr.s_addr)>>16)&255),
   (unsigned)((ntohl(from.sin_addr.s_addr)>>8)&255),
   (unsigned)(ntohl(from.sin_addr.s_addr)&255));
  bool found=false;
  for(size_t i=0;i<peers_.size();++i){
   if(peers_[i].ip==ip){
    peers_[i].name=name;
    peers_[i].titleId=id;
    peers_[i].lastSeen=GetTickCount();
    found=true;
    break;
   }
  }
  if(!found){
   LinkPeer p;
   p.ip=ip;
   p.name=name;
   p.titleId=id;
   p.lastSeen=GetTickCount();
   peers_.push_back(p);
  }
 }
 DWORD now=GetTickCount();
 for(size_t i=0;i<peers_.size();){
  if(now-peers_[i].lastSeen>15000)peers_.erase(peers_.begin()+i);
  else ++i;
 }
}

void SystemLinkService::poll(DWORD titleId,const std::string& title){
 DWORD now=GetTickCount();
 if(now-lastBeacon_>3000){
  sendBeacon(titleId,title);
  lastBeacon_=now;
 }
 receive();
}

void SystemLinkService::stop(){
 if(socket_!=INVALID_SOCKET){
  closesocket(socket_);
  socket_=INVALID_SOCKET;
 }
 peers_.clear();
}

BackgroundCoverQueue::BackgroundCoverQueue():
 runtime_(0),thread_(0),wake_(0),running_(false){
 InitializeCriticalSection(&lock_);
}

BackgroundCoverQueue::~BackgroundCoverQueue(){
 stop();
 DeleteCriticalSection(&lock_);
}

bool BackgroundCoverQueue::start(NativeRuntime* runtime){
 if(running_)return true;
 runtime_=runtime;
 wake_=CreateEvent(0,FALSE,FALSE,0);
 if(!wake_)return false;
 running_=true;
 thread_=CreateThread(0,0,threadProc,this,0,0);
 if(!thread_){
  running_=false;
  CloseHandle(wake_);
  wake_=0;
  return false;
 }
 return true;
}

void BackgroundCoverQueue::queue(size_t index){
 EnterCriticalSection(&lock_);
 for(size_t i=0;i<queue_.size();++i){
  if(queue_[i]==index){
   LeaveCriticalSection(&lock_);
   return;
  }
 }
 queue_.push_back(index);
 LeaveCriticalSection(&lock_);
 if(wake_)SetEvent(wake_);
}

DWORD WINAPI BackgroundCoverQueue::threadProc(LPVOID p){
 return ((BackgroundCoverQueue*)p)->work();
}

DWORD BackgroundCoverQueue::work(){
 while(running_){
  WaitForSingleObject(wake_,1000);
  if(!running_)break;
  size_t index=(size_t)-1;
  EnterCriticalSection(&lock_);
  if(!queue_.empty()){
   index=queue_[0];
   queue_.erase(queue_.begin());
  }
  LeaveCriticalSection(&lock_);
  if(index!=(size_t)-1&&runtime_)runtime_->refreshCover(index);
 }
 return 0;
}

void BackgroundCoverQueue::stop(){
 if(!running_)return;
 running_=false;
 if(wake_)SetEvent(wake_);
 if(thread_){
  WaitForSingleObject(thread_,3000);
  CloseHandle(thread_);
  thread_=0;
 }
 if(wake_){
  CloseHandle(wake_);
  wake_=0;
 }
 EnterCriticalSection(&lock_);
 queue_.clear();
 LeaveCriticalSection(&lock_);
 runtime_=0;
}

}
