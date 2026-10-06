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
 char line[1024];
 while(fgets(line,sizeof(line),f)){
  char* e=strpbrk(line,"\r\n");
  if(e)*e=0;
  if(!strncmp(line,"theme=",6)){
   setTheme(line+6);
   continue;
  }
  unsigned id=0,fav=0;
  unsigned long long last=0;
  char cat[64]={0};
  char title[512]={0};
  int fields=sscanf(line,"%8x|%u|%llu|%63[^|]|%511[^\n]",&id,&fav,&last,cat,title);
  if(fields>=4){
   TitleState s;
   s.favorite=fav!=0;
   s.lastPlayed=(ULONGLONG)last;
   s.category=cat;
   if(fields>=5)s.customTitle=title;
   states_[(DWORD)id]=s;
  }
 }
 fclose(f);
}

void StateStore::save() const {
 ensureDirs();
 FILE* f=fopen(path_.c_str(),"wb");
 if(!f)return;
 fprintf(f,"theme=%s\r\n",themeName_.c_str());
 for(std::map<DWORD,TitleState>::const_iterator it=states_.begin();it!=states_.end();++it){
  fprintf(f,"%08X|%u|%llu|%s|%s\r\n",
   (unsigned)it->first,
   it->second.favorite?1:0,
   (unsigned long long)it->second.lastPlayed,
   it->second.category.c_str(),
   it->second.customTitle.c_str());
 }
 fclose(f);
}

TitleState StateStore::state(DWORD id) const {
 std::map<DWORD,TitleState>::const_iterator it=states_.find(id);
 if(it==states_.end())return TitleState();
 return it->second;
}

void StateStore::toggleFavorite(DWORD id){
 states_[id].favorite=!states_[id].favorite;
 save();
}

void StateStore::recordPlayed(DWORD id){
 states_[id].lastPlayed=nowTicks();
 save();
}

void StateStore::setCategory(DWORD id,const std::string& c){
 states_[id].category=c;
 save();
}

void StateStore::setCustomTitle(DWORD id,const std::string& t){
 states_[id].customTitle=t;
 save();
}

void StateStore::apply(std::vector<TitleEntry>& titles) const {
 for(size_t i=0;i<titles.size();++i){
  TitleState s=state(titles[i].titleId);
  titles[i].favorite=s.favorite;
  titles[i].lastPlayed=s.lastPlayed;
  titles[i].category=s.category;
  if(!s.customTitle.empty())titles[i].title=s.customTitle;
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

void StateStore::setTheme(const std::string& n){
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
