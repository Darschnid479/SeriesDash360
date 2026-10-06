#include "NativeRuntime.hpp"
#include "NativeNetwork.hpp"
#include <algorithm>
#include <cstdio>

namespace sd360x {
static std::string join(const std::string&a,const std::string&b){ if(a.empty())return b; char c=a[a.size()-1]; return (c=='\\'||c=='/')?a+b:a+"\\"+b; }
static bool isDots(const char*n){return !strcmp(n,".")||!strcmp(n,"..");}

NativeRuntime::NativeRuntime():coverBase_("http://xboxunity.net/api/boxartfront/"){}

void NativeRuntime::scan(){ titles_.clear(); scanRoot("Hdd1:\\Games"); scanRoot("Usb0:\\Games"); scanRoot("Usb1:\\Games"); scanRoot("Hdd1:\\Homebrew"); scanRoot("Usb0:\\Homebrew"); }
void NativeRuntime::scanRoot(const char* root){ scanFolder(root,0); }

void NativeRuntime::scanFolder(const std::string& folder,int depth){
 if(depth>4)return;
 std::string xex=join(folder,"default.xex");
 DWORD attr=GetFileAttributesA(xex.c_str());
 if(attr!=INVALID_FILE_ATTRIBUTES && !(attr&FILE_ATTRIBUTE_DIRECTORY)){
   TitleEntry e; e.xex=xex; size_t p=folder.find_last_of("\\/"); e.title=(p==std::string::npos)?folder:folder.substr(p+1);
   e.titleId=readTitleId(xex.c_str()); if(folder.find("Homebrew")!=std::string::npos||folder.find("homebrew")!=std::string::npos)e.category="Homebrew";
   char id[16]; sprintf(id,"%08X",(unsigned)e.titleId); e.cover=std::string("Hdd1:\\SeriesDash360\\cache\\covers\\")+id+".jpg";
   titles_.push_back(e); return;
 }
 WIN32_FIND_DATAA fd; HANDLE h=FindFirstFileA(join(folder,"*").c_str(),&fd); if(h==INVALID_HANDLE_VALUE)return;
 do { if((fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)&&!isDots(fd.cFileName)) scanFolder(join(folder,fd.cFileName),depth+1); } while(FindNextFileA(h,&fd));
 FindClose(h);
}

DWORD NativeRuntime::readTitleId(const char* path) const {
 HANDLE h=CreateFileA(path,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0); if(h==INVALID_HANDLE_VALUE)return 0;
 BYTE hdr[24]; DWORD n=0; if(!ReadFile(h,hdr,sizeof(hdr),&n,0)||n!=sizeof(hdr)){CloseHandle(h);return 0;}
 if(hdr[0]!='X'||hdr[1]!='E'||hdr[2]!='X'||hdr[3]!='2'){CloseHandle(h);return 0;}
 DWORD count=(hdr[20]<<24)|(hdr[21]<<16)|(hdr[22]<<8)|hdr[23];
 for(DWORD i=0;i<count && i<4096;i++){ BYTE oh[8]; SetFilePointer(h,24+i*8,0,FILE_BEGIN); if(!ReadFile(h,oh,8,&n,0)||n!=8)break;
  DWORD key=(oh[0]<<24)|(oh[1]<<16)|(oh[2]<<8)|oh[3]; if(key!=0x00040006)continue;
  DWORD off=(oh[4]<<24)|(oh[5]<<16)|(oh[6]<<8)|oh[7]; BYTE ex[16]; SetFilePointer(h,off,0,FILE_BEGIN);
  if(ReadFile(h,ex,16,&n,0)&&n==16){DWORD id=(ex[12]<<24)|(ex[13]<<16)|(ex[14]<<8)|ex[15];CloseHandle(h);return id;}
 }
 CloseHandle(h); return 0;
}

bool NativeRuntime::launch(size_t i){ if(i>=titles_.size())return false; XLaunchNewImage(titles_[i].xex.c_str(),0); return false; }

SystemInfo NativeRuntime::systemInfo() const {
 SystemInfo s; BYTE in[16]={0x07},out[16]={0}; HalSendSMCMessage(in,out);
 s.cpu=out[1]; s.gpu=out[2]; s.edram=out[3]; s.mb=out[4];
 return s;
}

bool NativeRuntime::copyTree(const std::string& src,const std::string& dst){
 CreateDirectoryA(dst.c_str(),0);
 WIN32_FIND_DATAA fd; HANDLE h=FindFirstFileA(join(src,"*").c_str(),&fd); if(h==INVALID_HANDLE_VALUE)return false;
 bool ok=true; do{
  if(isDots(fd.cFileName))continue; std::string a=join(src,fd.cFileName),b=join(dst,fd.cFileName);
  if(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) ok=copyTree(a,b)&&ok;
  else if(!CopyFileA(a.c_str(),b.c_str(),FALSE)) ok=false;
 }while(FindNextFileA(h,&fd)); FindClose(h); return ok;
}
bool NativeRuntime::importDisc(const char* destination){ DWORD a=GetFileAttributesA("Dvd:\\"); if(a==INVALID_FILE_ATTRIBUTES)return false; return copyTree("Dvd:\\",destination); }

bool NativeRuntime::refreshCover(size_t i){
 if(i>=titles_.size()||titles_[i].titleId==0)return false; CreateDirectoryA("Hdd1:\\SeriesDash360",0);CreateDirectoryA("Hdd1:\\SeriesDash360\\cache",0);CreateDirectoryA("Hdd1:\\SeriesDash360\\cache\\covers",0);
 char id[16];sprintf(id,"%08X",(unsigned)titles_[i].titleId);
 std::string info="http://xboxunity.net/api/Covers/"; info+=id;
 std::string json; if(!httpGetText(info,json))return false;
 std::string key="\"front\":\""; size_t p=json.find(key); if(p==std::string::npos)return false; p+=key.size(); size_t q=json.find('"',p); if(q==std::string::npos)return false;
 std::string url=json.substr(p,q-p); return httpDownloadFile(url,titles_[i].cover);
}
}
