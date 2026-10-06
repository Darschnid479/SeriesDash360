#include "NativeNetwork.hpp"
#include <xtl.h>
#include <xhttp.h>
#include <cstdio>

namespace sd360x {
bool networkStart(){ XNetStartupParams xp; ZeroMemory(&xp,sizeof(xp)); xp.cfgSizeOfStruct=sizeof(xp); if(XNetStartup(&xp)!=0)return false; WSADATA w; return WSAStartup(MAKEWORD(2,2),&w)==0; }

static bool crack(const std::string& url,XHTTP_URL_COMPONENTS& c,char* host,DWORD hostLen,char* path,DWORD pathLen){
 ZeroMemory(&c,sizeof(c)); c.dwStructSize=sizeof(c); c.lpszHostName=host;c.dwHostNameLength=hostLen;c.lpszUrlPath=path;c.dwUrlPathLength=pathLen;
 return XHttpCrackUrl(url.c_str(),url.size(),0,&c)!=FALSE;
}
bool httpGetText(const std::string& url,std::string& out){
 out.clear(); char host[256]={0},path[1024]={0}; XHTTP_URL_COMPONENTS c; if(!crack(url,c,host,255,path,1023))return false;
 HINTERNET ses=XHttpOpen("SeriesDash360/1.0",XHTTP_ACCESS_TYPE_NO_PROXY,0,0,0); if(!ses)return false;
 DWORD flags=(c.nScheme==INTERNET_SCHEME_HTTPS)?XHTTP_FLAG_SECURE:0;
 HINTERNET con=XHttpConnect(ses,host,c.nPort,flags); if(!con){XHttpCloseHandle(ses);return false;}
 HINTERNET req=XHttpOpenRequest(con,"GET",path,0,0,0,0); bool ok=false;
 if(req && XHttpSendRequest(req,0,0,0,0,0,0) && XHttpReceiveResponse(req,0)){
   char buf[4096]; DWORD got=0; do{ got=0; if(!XHttpReadData(req,buf,sizeof(buf),&got))break; if(got)out.append(buf,got); }while(got); ok=!out.empty();
 }
 if(req)XHttpCloseHandle(req);XHttpCloseHandle(con);XHttpCloseHandle(ses);return ok;
}
bool httpDownloadFile(const std::string& url,const std::string& path){
 std::string data;if(!httpGetText(url,data))return false; HANDLE h=CreateFileA(path.c_str(),GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0);if(h==INVALID_HANDLE_VALUE)return false;
 DWORD n=0;BOOL ok=WriteFile(h,data.data(),data.size(),&n,0);CloseHandle(h);return ok&&n==data.size();
}
FtpServer::FtpServer():listen_(INVALID_SOCKET),client_(INVALID_SOCKET),cwd_("Hdd1:\\"){}
FtpServer::~FtpServer(){stop();}
bool FtpServer::start(unsigned short port){
 listen_=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);if(listen_==INVALID_SOCKET)return false;
 sockaddr_in a;ZeroMemory(&a,sizeof(a));a.sin_family=AF_INET;a.sin_addr.s_addr=INADDR_ANY;a.sin_port=htons(port);
 if(bind(listen_,(sockaddr*)&a,sizeof(a))==SOCKET_ERROR||listen(listen_,1)==SOCKET_ERROR){stop();return false;}
 u_long nb=1;ioctlsocket(listen_,FIONBIO,&nb);return true;
}
void FtpServer::poll(){
 if(listen_==INVALID_SOCKET)return;
 if(client_==INVALID_SOCKET){client_=accept(listen_,0,0);if(client_!=INVALID_SOCKET){u_long nb=1;ioctlsocket(client_,FIONBIO,&nb);const char*g="220 SeriesDash360 FTP ready\r\n";send(client_,g,strlen(g),0);}return;}
 char b[512];int n=recv(client_,b,sizeof(b)-1,0);if(n==0){closesocket(client_);client_=INVALID_SOCKET;return;}if(n<0)return;b[n]=0;command(b);
}
void FtpServer::command(const char* line){
 std::string s=line; while(!s.empty()&&(s[s.size()-1]=='\r'||s[s.size()-1]=='\n'))s.erase(s.size()-1);
 std::string r;
 if(s.find("USER")==0)r="331 Password not required\r\n"; else if(s.find("PASS")==0)r="230 Logged in\r\n"; else if(s=="SYST")r="215 XBOX360\r\n";
 else if(s=="PWD")r="257 \""+cwd_+"\"\r\n"; else if(s=="NOOP")r="200 OK\r\n"; else if(s=="QUIT"){r="221 Bye\r\n";send(client_,r.c_str(),r.size(),0);closesocket(client_);client_=INVALID_SOCKET;return;}
 else r="502 Command not implemented in 1.0 core\r\n";
 send(client_,r.c_str(),r.size(),0);
}
void FtpServer::stop(){if(client_!=INVALID_SOCKET){closesocket(client_);client_=INVALID_SOCKET;}if(listen_!=INVALID_SOCKET){closesocket(listen_);listen_=INVALID_SOCKET;}WSACleanup();XNetCleanup();}
}
