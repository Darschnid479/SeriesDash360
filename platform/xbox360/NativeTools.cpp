#include "NativeTools.hpp"
#include <cstdio>
#include <cstring>

namespace sd360x {

static std::string joinp(const std::string& a,const std::string& b){
 if(a.empty())return b;
 return a[a.size()-1]=='\\'?a+b:a+"\\"+b;
}

static bool dots(const char* n){
 return !strcmp(n,".")||!strcmp(n,"..");
}

NativeFileManager::NativeFileManager():path_("Hdd1:\\"){
 refresh();
}

void NativeFileManager::setPath(const std::string& p){
 path_=p;
 refresh();
}

void NativeFileManager::refresh(){
 items_.clear();
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA(joinp(path_,"*").c_str(),&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  if(dots(fd.cFileName))continue;
  FileEntry e;
  e.name=fd.cFileName;
  e.path=joinp(path_,e.name);
  e.directory=(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
  e.size=((ULONGLONG)fd.nFileSizeHigh<<32)|fd.nFileSizeLow;
  items_.push_back(e);
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

bool NativeFileManager::up(){
 size_t p=path_.find_last_of("\\");
 if(p==std::string::npos||p<5)return false;
 path_=path_.substr(0,p);
 refresh();
 return true;
}

bool NativeFileManager::enter(size_t i){
 if(i>=items_.size()||!items_[i].directory)return false;
 path_=items_[i].path;
 refresh();
 return true;
}

bool NativeFileManager::removeTree(const std::string& path){
 DWORD attr=GetFileAttributesA(path.c_str());
 if(attr==INVALID_FILE_ATTRIBUTES)return false;
 if(!(attr&FILE_ATTRIBUTE_DIRECTORY))return DeleteFileA(path.c_str())!=FALSE;
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA(joinp(path,"*").c_str(),&fd);
 if(h!=INVALID_HANDLE_VALUE){
  do{
   if(dots(fd.cFileName))continue;
   removeTree(joinp(path,fd.cFileName));
  }while(FindNextFileA(h,&fd));
  FindClose(h);
 }
 return RemoveDirectoryA(path.c_str())!=FALSE;
}

bool NativeFileManager::remove(size_t i){
 if(i>=items_.size())return false;
 bool ok=removeTree(items_[i].path);
 refresh();
 return ok;
}

bool NativeFileManager::createFolder(const std::string& n){
 BOOL ok=CreateDirectoryA(joinp(path_,n).c_str(),0);
 refresh();
 return ok!=FALSE;
}

bool NativeFileManager::copyTree(const std::string& s,const std::string& d){
 DWORD attr=GetFileAttributesA(s.c_str());
 if(attr==INVALID_FILE_ATTRIBUTES)return false;
 if(!(attr&FILE_ATTRIBUTE_DIRECTORY)){
  return CopyFileA(s.c_str(),d.c_str(),FALSE)!=FALSE;
 }
 CreateDirectoryA(d.c_str(),0);
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA(joinp(s,"*").c_str(),&fd);
 if(h==INVALID_HANDLE_VALUE)return false;
 bool ok=true;
 do{
  if(dots(fd.cFileName))continue;
  ok=copyTree(joinp(s,fd.cFileName),joinp(d,fd.cFileName))&&ok;
 }while(FindNextFileA(h,&fd));
 FindClose(h);
 return ok;
}

bool NativeFileManager::copy(size_t i,const std::string& d){
 if(i>=items_.size())return false;
 return copyTree(items_[i].path,joinp(d,items_[i].name));
}

bool NativeFileManager::move(size_t i,const std::string& d){
 if(i>=items_.size())return false;
 BOOL ok=MoveFileA(items_[i].path.c_str(),joinp(d,items_[i].name).c_str());
 refresh();
 return ok!=FALSE;
}

void TitleUpdateManager::scanFolder(const std::string& p){
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA(joinp(p,"*").c_str(),&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  if(dots(fd.cFileName))continue;
  std::string full=joinp(p,fd.cFileName);
  if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
   scanFolder(full);
  }else{
   TitleUpdate t;
   t.path=full;
   t.name=fd.cFileName;
   t.enabled=t.name.find(".disabled")==std::string::npos;
   items_.push_back(t);
  }
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

void TitleUpdateManager::scan(DWORD id){
 items_.clear();
 char tid[16];
 sprintf(tid,"%08X",(unsigned)id);
 scanFolder(std::string("Hdd1:\\Content\\0000000000000000\\")+tid+"\\000B0000");
 scanFolder("Hdd1:\\Cache");
}

bool TitleUpdateManager::setEnabled(size_t i,bool en){
 if(i>=items_.size())return false;
 TitleUpdate& t=items_[i];
 if(t.enabled==en)return true;
 std::string n=t.path;
 if(en){
  size_t p=n.rfind(".disabled");
  if(p!=std::string::npos)n.erase(p);
 }else{
  n+=".disabled";
 }
 BOOL ok=MoveFileA(t.path.c_str(),n.c_str());
 if(ok){
  t.path=n;
  size_t slash=n.find_last_of("\\");
  t.name=slash==std::string::npos?n:n.substr(slash+1);
  t.enabled=en;
 }
 return ok!=FALSE;
}

void SaveBrowser::scanTree(const std::string& p,const std::string& profile,int depth){
 if(depth>4)return;
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA(joinp(p,"*").c_str(),&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  if(dots(fd.cFileName))continue;
  std::string full=joinp(p,fd.cFileName);
  if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY){
   scanTree(full,profile,depth+1);
  }else{
   SaveEntry s;
   s.profile=profile;
   s.path=full;
   s.name=fd.cFileName;
   s.size=((ULONGLONG)fd.nFileSizeHigh<<32)|fd.nFileSizeLow;
   items_.push_back(s);
  }
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

void SaveBrowser::scan(DWORD id){
 items_.clear();
 char tid[16];
 sprintf(tid,"%08X",(unsigned)id);
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA("Hdd1:\\Content\\*",&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)||dots(fd.cFileName)||!strcmp(fd.cFileName,"0000000000000000"))continue;
  std::string root=std::string("Hdd1:\\Content\\")+fd.cFileName+"\\"+tid+"\\00000001";
  scanTree(root,fd.cFileName,0);
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

static std::string narrow(const wchar_t* w){
 if(!w)return "";
 char b[512];
 int n=WideCharToMultiByte(CP_UTF8,0,w,-1,b,sizeof(b),0,0);
 return n>0?std::string(b):std::string();
}

bool AchievementBrowser::load(DWORD titleId,DWORD user){
 items_.clear();
 XUID xuid=0;
 if(XUserGetXUID(user,&xuid)!=ERROR_SUCCESS)return false;
 DWORD bytes=0;
 HANDLE e=INVALID_HANDLE_VALUE;
 DWORD r=XUserCreateAchievementEnumerator(titleId,user,xuid,XACHIEVEMENT_DETAILS_ALL,0,256,&bytes,&e);
 if(r!=ERROR_SUCCESS||!bytes||e==INVALID_HANDLE_VALUE){
  if(e!=INVALID_HANDLE_VALUE)CloseHandle(e);
  return false;
 }
 BYTE* buf=new BYTE[bytes];
 DWORD count=0;
 r=XEnumerate(e,buf,bytes,&count,0);
 CloseHandle(e);
 if(r!=ERROR_SUCCESS){
  delete[] buf;
  return false;
 }
 XACHIEVEMENT_DETAILS* a=(XACHIEVEMENT_DETAILS*)buf;
 for(DWORD i=0;i<count;i++){
  AchievementEntry o;
  o.id=a[i].dwId;
  o.score=a[i].dwCred;
  o.flags=a[i].dwFlags;
  o.label=narrow(a[i].pwszLabel);
  o.description=narrow(a[i].pwszDescription);
  items_.push_back(o);
 }
 delete[] buf;
 return true;
}

}
