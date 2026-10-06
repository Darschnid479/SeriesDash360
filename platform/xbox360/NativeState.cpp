#include "NativeState.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>

namespace sd360x {

static void ensureDirs(){
 CreateDirectoryA("Hdd1:\\SeriesDash360",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\userdata",0);
}

static std::string lowerCopy(std::string s){
 for(size_t i=0;i<s.size();++i){
  if(s[i]>='A'&&s[i]<='Z')s[i]=(char)(s[i]-'A'+'a');
 }
 return s;
}

static ULONGLONG nowTicks(){
 FILETIME ft;
 GetSystemTimeAsFileTime(&ft);
 return ((ULONGLONG)ft.dwHighDateTime<<32)|ft.dwLowDateTime;
}

static std::vector<std::string> split(const std::string& s,char delim){
 std::vector<std::string> out;
 size_t start=0;
 for(;;){
  size_t p=s.find(delim,start);
  if(p==std::string::npos){out.push_back(s.substr(start));break;}
  out.push_back(s.substr(start,p-start));
  start=p+1;
 }
 return out;
}

static std::string cleanField(std::string s){
 for(size_t i=0;i<s.size();++i){
  if(s[i]=='|'||s[i]=='\r'||s[i]=='\n')s[i]=' ';
 }
 return s;
}

StateStore::StateStore():
 themeName_("series-dark"),
 path_("Hdd1:\\SeriesDash360\\userdata\\state.ini"){
 setTheme(themeName_);
}

void StateStore::load(){
 states_.clear();
 ensureDirs();
 FILE* f=fopen(path_.c_str(),"rb");
 if(!f)return;
 char line[4096];
 while(fgets(line,sizeof(line),f)){
  char* e=strpbrk(line,"\r\n");
  if(e)*e=0;
  if(!strncmp(line,"theme=",6)){
   setTheme(line+6);
   continue;
  }
  std::vector<std::string> p=split(line,'|');
  if(p.size()<4)continue;
  DWORD id=(DWORD)strtoul(p[0].c_str(),0,16);
  TitleState s;
  s.favorite=atoi(p[1].c_str())!=0;
  s.lastPlayed=(ULONGLONG)_strtoui64(p[2].c_str(),0,10);
  s.category=p[3];
  if(p.size()>4)s.customTitle=p[4];
  if(p.size()>5)s.developer=p[5];
  if(p.size()>6)s.genre=p[6];
  if(p.size()>7)s.year=p[7];
  if(p.size()>8)s.description=p[8];
  if(p.size()>9)s.coverOverride=p[9];
  states_[id]=s;
 }
 fclose(f);
}

void StateStore::save() const {
 ensureDirs();
 FILE* f=fopen(path_.c_str(),"wb");
 if(!f)return;
 fprintf(f,"theme=%s\r\n",themeName_.c_str());
 for(std::map<DWORD,TitleState>::const_iterator it=states_.begin();it!=states_.end();++it){
  const TitleState& s=it->second;
  fprintf(f,"%08X|%u|%llu|%s|%s|%s|%s|%s|%s|%s\r\n",
   (unsigned)it->first,
   s.favorite?1:0,
   (unsigned long long)s.lastPlayed,
   cleanField(s.category).c_str(),
   cleanField(s.customTitle).c_str(),
   cleanField(s.developer).c_str(),
   cleanField(s.genre).c_str(),
   cleanField(s.year).c_str(),
   cleanField(s.description).c_str(),
   cleanField(s.coverOverride).c_str());
 }
 fclose(f);
}

TitleState StateStore::state(DWORD id) const {
 std::map<DWORD,TitleState>::const_iterator it=states_.find(id);
 if(it==states_.end())return TitleState();
 return it->second;
}

void StateStore::toggleFavorite(DWORD id){states_[id].favorite=!states_[id].favorite;save();}
void StateStore::recordPlayed(DWORD id){states_[id].lastPlayed=nowTicks();save();}
void StateStore::setCategory(DWORD id,const std::string& c){states_[id].category=c;save();}
void StateStore::setCustomTitle(DWORD id,const std::string& t){states_[id].customTitle=t;save();}
void StateStore::setDeveloper(DWORD id,const std::string& v){states_[id].developer=v;save();}
void StateStore::setGenre(DWORD id,const std::string& v){states_[id].genre=v;save();}
void StateStore::setYear(DWORD id,const std::string& v){states_[id].year=v;save();}
void StateStore::setDescription(DWORD id,const std::string& v){states_[id].description=v;save();}
void StateStore::setCoverOverride(DWORD id,const std::string& v){states_[id].coverOverride=v;save();}

void StateStore::apply(std::vector<TitleEntry>& titles) const {
 for(size_t i=0;i<titles.size();++i){
  TitleState s=state(titles[i].titleId);
  titles[i].favorite=s.favorite;
  titles[i].lastPlayed=s.lastPlayed;
  titles[i].category=s.category;
  if(!s.customTitle.empty())titles[i].title=s.customTitle;
  if(!s.coverOverride.empty())titles[i].cover=s.coverOverride;
 }
}

struct RecentSorter {
 const std::vector<TitleEntry>* titles;
 const StateStore* store;
 RecentSorter(const std::vector<TitleEntry>& t,const StateStore& s):titles(&t),store(&s){}
 bool operator()(size_t a,size_t b) const {
  return store->state((*titles)[a].titleId).lastPlayed >
         store->state((*titles)[b].titleId).lastPlayed;
 }
};

std::vector<size_t> StateStore::filter(const std::vector<TitleEntry>& titles,const std::string& query,LibraryFilter f) const {
 std::vector<size_t> out;
 std::string q=lowerCopy(query);
 for(size_t i=0;i<titles.size();++i){
  TitleState s=state(titles[i].titleId);
  if(!q.empty()&&lowerCopy(titles[i].title).find(q)==std::string::npos)continue;
  if(f==FILTER_FAVORITES&&!s.favorite)continue;
  if(f==FILTER_RECENT&&s.lastPlayed==0)continue;
  if(f==FILTER_GAMES&&s.category!="Games")continue;
  if(f==FILTER_HOMEBREW&&s.category!="Homebrew")continue;
  out.push_back(i);
 }
 if(f==FILTER_RECENT)std::sort(out.begin(),out.end(),RecentSorter(titles,*this));
 return out;
}

static DWORD parseColor(const std::string& s,DWORD fallback){
 if(s.empty())return fallback;
 const char* p=s.c_str();
 if(!strncmp(p,"0x",2)||!strncmp(p,"0X",2))p+=2;
 unsigned long v=strtoul(p,0,16);
 if(strlen(p)<=6)v|=0xFF000000;
 return (DWORD)v;
}

bool StateStore::loadThemeFile(const std::string& path){
 FILE* f=fopen(path.c_str(),"rb");
 if(!f)return false;
 Theme t;
 char line[1024];
 while(fgets(line,sizeof(line),f)){
  char* e=strpbrk(line,"\r\n");if(e)*e=0;
  char* eq=strchr(line,'=');if(!eq)continue;*eq=0;
  std::string k=line,v=eq+1;
  if(k=="background")t.background=parseColor(v,t.background);
  else if(k=="panel")t.panel=parseColor(v,t.panel);
  else if(k=="accent")t.accent=parseColor(v,t.accent);
  else if(k=="text")t.text=parseColor(v,t.text);
  else if(k=="muted")t.muted=parseColor(v,t.muted);
  else if(k=="background_image")t.backgroundImage=v;
 }
 fclose(f);
 theme_=t;
 themeName_=std::string("file:")+path;
 return true;
}

void StateStore::setTheme(const std::string& n){
 if(n.size()>5&&n.substr(0,5)=="file:"){
  if(loadThemeFile(n.substr(5)))return;
 }
 themeName_=n;
 theme_=Theme();
 if(n=="xbox-green"){
  theme_.background=0xFF071008;
  theme_.panel=0xFF102416;
  theme_.accent=0xFF52B043;
 }else if(n=="series-blue"){
  theme_.background=0xFF080B12;
  theme_.panel=0xFF111B2D;
  theme_.accent=0xFF2878D8;
 }else if(n=="oled"){
  theme_.background=0xFF000000;
  theme_.panel=0xFF0A0A0A;
  theme_.accent=0xFF107C10;
 }
}

bool showKeyboard(const wchar_t* title,const wchar_t* desc,std::string& result){
 wchar_t buf[256];
 ZeroMemory(buf,sizeof(buf));
 XOVERLAPPED ov;
 ZeroMemory(&ov,sizeof(ov));
 DWORD r=XShowKeyboardUI(0,0,L"",title,desc,buf,256,&ov);
 if(r!=ERROR_IO_PENDING&&r!=ERROR_SUCCESS)return false;
 DWORD ignored=0;
 if(XGetOverlappedResult(&ov,&ignored,TRUE)!=ERROR_SUCCESS)return false;
 char out[512];
 int n=WideCharToMultiByte(CP_UTF8,0,buf,-1,out,sizeof(out),0,0);
 if(n<=0)return false;
 result=out;
 return true;
}

}
