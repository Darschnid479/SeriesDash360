#include "NativeRuntime.hpp"
#include "NativeRenderer.hpp"
#include "NativeNetwork.hpp"
#include "NativeState.hpp"
#include "NativeTools.hpp"
#include "NativeExtensions.hpp"

#include <xtl.h>
#include <cstdio>
#include <string>
#include <vector>

using namespace sd360x;

enum Page {
 PAGE_LIBRARY=0,
 PAGE_TOOLS,
 PAGE_FILES,
 PAGE_UPDATES,
 PAGE_ACHIEVEMENTS,
 PAGE_SAVES,
 PAGE_METADATA,
 PAGE_THEMES,
 PAGE_PLUGINS,
 PAGE_LINK,
 PAGE_SYSTEM
};

static bool pressed(WORD now,WORD old,WORD key){
 return (now&key)&&!(old&key);
}

static const char* filterName(LibraryFilter f){
 switch(f){
  case FILTER_GAMES:return "Games";
  case FILTER_HOMEBREW:return "Homebrew";
  case FILTER_FAVORITES:return "Favorites";
  case FILTER_RECENT:return "Recent";
  default:return "All";
 }
}

static void ensureUserFolders(){
 CreateDirectoryA("Hdd1:\\SeriesDash360",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\screenshots",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\plugins",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\scripts",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\cache",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\cache\\covers",0);
 CreateDirectoryA("Hdd1:\\SeriesDash360\\userdata",0);\n CreateDirectoryA("Hdd1:\\SeriesDash360\\themes",0);
}

static void scanThemeChoices(std::vector<std::string>& out){
 out.clear();
 out.push_back("series-dark");
 out.push_back("xbox-green");
 out.push_back("series-blue");
 out.push_back("oled");
 WIN32_FIND_DATAA fd;
 HANDLE h=FindFirstFileA("Hdd1:\\SeriesDash360\\themes\\*.theme",&fd);
 if(h==INVALID_HANDLE_VALUE)return;
 do{
  out.push_back(std::string("file:Hdd1:\\SeriesDash360\\themes\\")+fd.cFileName);
 }while(FindNextFileA(h,&fd));
 FindClose(h);
}

static std::string themeLabel(const std::string& value){
 if(value.size()>5&&value.substr(0,5)=="file:"){
  size_t p=value.find_last_of("\\/");
  return p==std::string::npos?value.substr(5):value.substr(p+1);
 }
 return value;
}

static DWORD currentTitleIndex(const std::vector<size_t>& visible,size_t selected){
 if(visible.empty())return 0xFFFFFFFF;
 if(selected>=visible.size())selected=visible.size()-1;
 return (DWORD)visible[selected];
}

static void drawList(NativeRenderer& ui,const std::vector<std::string>& lines,size_t selected,const Theme& t){
 size_t first=selected>8?selected-8:0;
 for(size_t i=first;i<lines.size()&&i<first+10;++i){
  float y=130.0f+(float)(i-first)*48.0f;
  if(i==selected)ui.rect(34,y-6,1170,42,t.accent);
  ui.text(52,y,t.text,lines[i]);
 }
}

int main(){
 ensureUserFolders();

 NativeRenderer ui;
 if(!ui.init())return 2;

 networkStart();
 FtpServer ftp;
 ftp.start(7564);

 NativeRuntime runtime;
 StateStore state;
 state.load();
 runtime.scan();
 state.apply(runtime.titles());

 PluginManager plugins;
 plugins.scan();

 ScriptRuntime scripts;
 scripts.runStartupScripts(runtime,state);
 state.apply(runtime.titles());

 BackgroundCoverQueue covers;
 covers.start(&runtime);
 for(size_t i=0;i<runtime.titles().size();++i){
  if(GetFileAttributesA(runtime.titles()[i].cover.c_str())==INVALID_FILE_ATTRIBUTES)covers.queue(i);
 }

 SystemLinkService link;
 link.start();

 NativeFileManager files;
 TitleUpdateManager updates;
 AchievementBrowser achievements;
 SaveBrowser saves;

 Page page=PAGE_LIBRARY;
 LibraryFilter filter=FILTER_ALL;
 std::string query;
 size_t selected=0;
 size_t row=0;
 WORD oldButtons=0;
 bool running=true;
 bool captureRequested=false;

 const char* tools[]={
  "File Manager",
  "Title Updates",
  "Achievements",
  "Save Games",
  "Metadata Editor",
  "Themes / Skins",
  "Plugins / Scripts",
  "System-Link",
  "Copy Disc to HDD",
  "System"
 };
 const size_t toolCount=sizeof(tools)/sizeof(tools[0]);
 const char* themeNames[]={"series-dark","xbox-green","series-blue","oled"};
 const size_t themeCount=sizeof(themeNames)/sizeof(themeNames[0]);
 size_t themeSelected=0;

 while(running){
  ftp.poll();

  std::vector<size_t> visible=state.filter(runtime.titles(),query,filter);
  if(selected>=visible.size()&&selected>0)selected=visible.empty()?0:visible.size()-1;
  DWORD titleIndex=currentTitleIndex(visible,selected);
  DWORD activeTitleId=0;
  std::string activeTitle="SeriesDash360";
  if(titleIndex!=0xFFFFFFFF&&titleIndex<runtime.titles().size()){
   activeTitleId=runtime.titles()[titleIndex].titleId;
   activeTitle=runtime.titles()[titleIndex].title;
  }
  link.poll(activeTitleId,activeTitle);

  XINPUT_STATE st;
  ZeroMemory(&st,sizeof(st));
  WORD b=0;
  if(XInputGetState(0,&st)==ERROR_SUCCESS)b=st.Gamepad.wButtons;

  if(pressed(b,oldButtons,XINPUT_GAMEPAD_RIGHT_THUMB))captureRequested=true;

  if(page==PAGE_LIBRARY){
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_RIGHT)&&selected+1<visible.size())selected++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_LEFT)&&selected>0)selected--;

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_LEFT_SHOULDER)){
    std::string q;
    if(showKeyboard(L"Search",L"Search games, homebrew and apps",q)){
     query=q;
     selected=0;
    }
   }

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_RIGHT_SHOULDER)){
    filter=(LibraryFilter)(((int)filter+1)%5);
    selected=0;
   }

   if(titleIndex!=0xFFFFFFFF){
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)){
     state.recordPlayed(activeTitleId);
     runtime.launch(titleIndex);
    }
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_X)){
     state.toggleFavorite(activeTitleId);
     state.apply(runtime.titles());
    }
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y))covers.queue(titleIndex);
   }

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){
    query.clear();
    filter=FILTER_ALL;
    selected=0;
   }

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_START)){
    page=PAGE_TOOLS;
    row=0;
   }

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_BACK))running=false;
  }
  else if(page==PAGE_TOOLS){
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<toolCount)row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)){
    if(row==0){
     files.setPath("Hdd1:\\");
     page=PAGE_FILES;
     row=0;
    }else if(row==1){
     if(activeTitleId)updates.scan(activeTitleId);
     page=PAGE_UPDATES;
     row=0;
    }else if(row==2){
     if(activeTitleId)achievements.load(activeTitleId);
     page=PAGE_ACHIEVEMENTS;
     row=0;
    }else if(row==3){
     if(activeTitleId)saves.scan(activeTitleId);
     page=PAGE_SAVES;
     row=0;
    }else if(row==4){
     page=PAGE_METADATA;
     row=0;
    }else if(row==5){
     scanThemeChoices(themeNames); for(size_t i=0;i<themeNames.size();i++)if(state.themeName()==themeNames[i])themeSelected=i;
     page=PAGE_THEMES;
     row=0;
    }else if(row==6){
     plugins.scan();
     page=PAGE_PLUGINS;
     row=0;
    }else if(row==7){
     page=PAGE_LINK;
     row=0;
    }else if(row==8){
     std::string destination="Hdd1:\\Games\\DiscImport";
     std::string entered;
     if(showKeyboard(L"Disc destination",L"Destination path on HDD",entered)&&!entered.empty())destination=entered;
     runtime.importDisc(destination.c_str());
     runtime.scan();
     state.apply(runtime.titles());
     selected=0;
    }else if(row==9){
     page=PAGE_SYSTEM;
     row=0;
    }
   }

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B))page=PAGE_LIBRARY;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_BACK))running=false;
  }
  else if(page==PAGE_FILES){
   const std::vector<FileEntry>& list=files.items();
   if(row>=list.size()&&row>0)row=list.empty()?0:list.size()-1;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<list.size())row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;

   if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)&&row<list.size()){
    if(files.enter(row))row=0;
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){
    if(!files.up()){page=PAGE_TOOLS;row=0;}
    else row=0;
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_X)&&row<list.size()){
    files.remove(row);
    row=0;
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)){
    std::string name;
    if(showKeyboard(L"New folder",L"Enter folder name",name)&&!name.empty())files.createFolder(name);
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_LEFT_SHOULDER)&&row<list.size()){
    std::string dest;
    if(showKeyboard(L"Copy to",L"Enter destination directory",dest)&&!dest.empty())files.copy(row,dest);
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_RIGHT_SHOULDER)&&row<list.size()){
    std::string dest;
    if(showKeyboard(L"Move to",L"Enter destination directory",dest)&&!dest.empty())files.move(row,dest);
   }
  }
  else if(page==PAGE_UPDATES){
   const std::vector<TitleUpdate>& list=updates.items();
   if(row>=list.size()&&row>0)row=list.empty()?0:list.size()-1;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<list.size())row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)&&row<list.size())updates.setEnabled(row,!list[row].enabled);
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)&&activeTitleId)updates.scan(activeTitleId);
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_ACHIEVEMENTS){
   const std::vector<AchievementEntry>& list=achievements.items();
   if(row>=list.size()&&row>0)row=list.empty()?0:list.size()-1;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<list.size())row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)&&activeTitleId)achievements.load(activeTitleId);
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_SAVES){
   const std::vector<SaveEntry>& list=saves.items();
   if(row>=list.size()&&row>0)row=list.empty()?0:list.size()-1;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<list.size())row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)&&activeTitleId)saves.scan(activeTitleId);
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_METADATA){
   if(titleIndex!=0xFFFFFFFF){
    std::string v;
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)&&showKeyboard(L"Game title",L"Custom display title",v)){
     state.setCustomTitle(activeTitleId,v);state.apply(runtime.titles());
    }
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_X)&&showKeyboard(L"Developer",L"Developer / publisher",v))state.setDeveloper(activeTitleId,v);
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)&&showKeyboard(L"Genre",L"Genre",v))state.setGenre(activeTitleId,v);
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_LEFT_SHOULDER)&&showKeyboard(L"Year",L"Release year",v))state.setYear(activeTitleId,v);
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_RIGHT_SHOULDER)&&showKeyboard(L"Description",L"Game description",v))state.setDescription(activeTitleId,v);
    if(pressed(b,oldButtons,XINPUT_GAMEPAD_LEFT_THUMB)){
     TitleState ts=state.state(activeTitleId);
     const char* cats[]={"Games","Homebrew","Emulators","Apps"};
     size_t ci=0;
     for(size_t i=0;i<4;i++)if(ts.category==cats[i])ci=i;
     ci=(ci+1)%4;
     state.setCategory(activeTitleId,cats[ci]);
     state.apply(runtime.titles());
    }
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_THEMES){
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_RIGHT))if(!themeNames.empty())themeSelected=(themeSelected+1)%themeNames.size();
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_LEFT))if(!themeNames.empty())themeSelected=(themeSelected+themeNames.size()-1)%themeNames.size();
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)){
    if(!themeNames.empty())state.setTheme(themeNames[themeSelected]);
    state.save();
    ui.setBackground(state.theme().background);
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_PLUGINS){
   const std::vector<PluginInfo>& list=plugins.items();
   if(row>=list.size()&&row>0)row=list.empty()?0:list.size()-1;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_DOWN)&&row+1<list.size())row++;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_UP)&&row>0)row--;
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)&&row<list.size())plugins.load(row);
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_X)){
    scripts.runStartupScripts(runtime,state);
    state.apply(runtime.titles());
   }
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)){plugins.scan();row=0;}
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_LINK){
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }
  else if(page==PAGE_SYSTEM){
   if(pressed(b,oldButtons,XINPUT_GAMEPAD_B)){page=PAGE_TOOLS;row=0;}
  }

  oldButtons=b;

  const Theme& theme=state.theme();
  ui.setBackground(theme.background);
  ui.begin();
  ui.text(34,24,theme.text,"SERIESDASH360  1.0");
  ui.text(1010,24,theme.muted,"R3 Screenshot");

  if(page==PAGE_LIBRARY){
   char head[256];
   sprintf(head,"My Games   Filter: %s   Search: %s",filterName(filter),query.empty()?"-":query.c_str());
   ui.text(34,78,theme.text,head);

   size_t first=(selected/6)*6;
   for(size_t slot=0;slot<6;slot++){
    size_t vi=first+slot;
    if(vi>=visible.size())break;
    size_t n=visible[vi];
    float x=34.0f+(float)slot*202.0f;
    DWORD frame=(vi==selected)?theme.accent:theme.panel;
    ui.rect(x-4,124,196,310,frame);
    ui.cover(x,128,188,250,runtime.titles()[n].cover,0xFF1C4028);
    ui.rect(x,378,188,52,0xE8000000);
    ui.text(x+8,390,theme.text,runtime.titles()[n].title);
    if(runtime.titles()[n].favorite)ui.text(x+160,136,0xFFFFD700,"*");
   }
   char count[64];
   sprintf(count,"%u / %u titles",(unsigned)visible.size(),(unsigned)runtime.titles().size());
   ui.text(34,466,theme.muted,count);
   ui.text(34,520,theme.muted,"A Launch   X Favorite   Y Cover   LB Search   RB Filter   START Tools   B Clear");
  }
  else if(page==PAGE_TOOLS){
   ui.text(34,78,theme.text,"Tools");
   std::vector<std::string> lines;
   for(size_t i=0;i<toolCount;i++)lines.push_back(tools[i]);
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"A Open   B Library");
  }
  else if(page==PAGE_FILES){
   ui.text(34,78,theme.text,std::string("File Manager   ")+files.path());
   std::vector<std::string> lines;
   const std::vector<FileEntry>& list=files.items();
   for(size_t i=0;i<list.size();++i){
    char size[64];
    if(list[i].directory)sprintf(size,"[DIR] %s",list[i].name.c_str());
    else sprintf(size,"%s  (%llu bytes)",list[i].name.c_str(),(unsigned long long)list[i].size);
    lines.push_back(size);
   }
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"A Enter   B Up   X Delete   Y New Folder   LB Copy   RB Move");
  }
  else if(page==PAGE_UPDATES){
   ui.text(34,78,theme.text,"Title Updates");
   std::vector<std::string> lines;
   const std::vector<TitleUpdate>& list=updates.items();
   for(size_t i=0;i<list.size();++i)lines.push_back(std::string(list[i].enabled?"[ON]  ":"[OFF] ")+list[i].name);
   if(lines.empty())lines.push_back("No title updates found");
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"A Enable/Disable   Y Rescan   B Back");
  }
  else if(page==PAGE_ACHIEVEMENTS){
   ui.text(34,78,theme.text,"Achievements");
   std::vector<std::string> lines;
   const std::vector<AchievementEntry>& list=achievements.items();
   for(size_t i=0;i<list.size();++i){
    char s[768];
    sprintf(s,"%s  -  %uG%s",list[i].label.c_str(),(unsigned)list[i].score,(list[i].flags&XACHIEVEMENT_DETAILS_ACHIEVED)?"  [Unlocked]":"");
    lines.push_back(s);
   }
   if(lines.empty())lines.push_back("No achievement data available for this profile/title");
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"Y Refresh   B Back");
  }
  else if(page==PAGE_SAVES){
   ui.text(34,78,theme.text,"Save Games");
   std::vector<std::string> lines;
   const std::vector<SaveEntry>& list=saves.items();
   for(size_t i=0;i<list.size();++i){
    char s[768];
    sprintf(s,"%s  [%s]  %llu bytes",list[i].name.c_str(),list[i].profile.c_str(),(unsigned long long)list[i].size);
    lines.push_back(s);
   }
   if(lines.empty())lines.push_back("No save files found");
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"Y Refresh   B Back");
  }
  else if(page==PAGE_METADATA){
   ui.text(34,78,theme.text,"Metadata Editor");
   if(titleIndex!=0xFFFFFFFF){
    TitleState ts=state.state(activeTitleId);
    char id[64];
    sprintf(id,"Title ID: %08X",(unsigned)activeTitleId);
    ui.text(34,130,theme.text,runtime.titles()[titleIndex].title);
    ui.text(34,170,theme.muted,id);
    ui.text(34,210,theme.text,std::string("Category: ")+ts.category);
    ui.text(34,250,theme.text,std::string("Developer: ")+ts.developer);
    ui.text(34,290,theme.text,std::string("Genre: ")+ts.genre);
    ui.text(34,330,theme.text,std::string("Year: ")+ts.year);
    ui.text(34,370,theme.text,std::string("Description: ")+ts.description);
    ui.text(34,450,theme.muted,"A Title   X Developer   Y Genre   LB Year   RB Description   L3 Category");
   }else ui.text(34,130,theme.muted,"Select a title in the library first.");
   ui.text(34,650,theme.muted,"B Back");
  }
  else if(page==PAGE_THEMES){
   ui.text(34,78,theme.text,"Themes / Skins");
   for(size_t i=0;i<themeCount;i++){
    float x=34.0f+(float)i*290.0f;
    ui.rect(x,140,270,140,i==themeSelected?theme.accent:theme.panel);
    ui.text(x+18,190,theme.text,themeNames[i]);
   }
   ui.text(34,330,theme.muted,"D-pad Left/Right select   A Apply   B Back");
  }
  else if(page==PAGE_PLUGINS){
   ui.text(34,78,theme.text,"Plugins / Scripts");
   std::vector<std::string> lines;
   const std::vector<PluginInfo>& list=plugins.items();
   for(size_t i=0;i<list.size();++i)lines.push_back(std::string(list[i].loaded?"[LOADED] ":"[READY] ")+list[i].name);
   if(lines.empty())lines.push_back("No plugins found in Hdd1:\\SeriesDash360\\plugins");
   drawList(ui,lines,row,theme);
   ui.text(34,650,theme.muted,"A Load Plugin   X Run .sd360 Scripts   Y Rescan   B Back");
  }
  else if(page==PAGE_LINK){
   ui.text(34,78,theme.text,"System-Link Discovery");
   std::vector<std::string> lines;
   const std::vector<LinkPeer>& peers=link.peers();
   for(size_t i=0;i<peers.size();++i){
    char s[512];
    sprintf(s,"%s   %s   Title %08X",peers[i].name.c_str(),peers[i].ip.c_str(),(unsigned)peers[i].titleId);
    lines.push_back(s);
   }
   if(lines.empty())lines.push_back("No SeriesDash360/System-Link peers discovered on LAN");
   drawList(ui,lines,0,theme);
   ui.text(34,650,theme.muted,"LAN discovery active on UDP 30720   B Back");
  }
  else if(page==PAGE_SYSTEM){
   SystemInfo s=runtime.systemInfo();
   char line[128];
   ui.text(34,78,theme.text,"System");
   ui.rect(34,135,280,150,theme.panel);sprintf(line,"CPU  %d C",s.cpu);ui.text(54,170,theme.text,line);
   ui.rect(330,135,280,150,theme.panel);sprintf(line,"GPU  %d C",s.gpu);ui.text(350,170,theme.text,line);
   ui.rect(626,135,280,150,theme.panel);sprintf(line,"EDRAM  %d C",s.edram);ui.text(646,170,theme.text,line);
   ui.rect(922,135,280,150,theme.panel);sprintf(line,"BOARD  %d C",s.mb);ui.text(942,170,theme.text,line);
   ui.text(34,340,theme.muted,"FTP :7564   Background covers active   System-Link discovery active");
   ui.text(34,650,theme.muted,"B Back");
  }

  ui.end();

  if(captureRequested){
   char path[256];
   SYSTEMTIME stime;
   GetLocalTime(&stime);
   sprintf(path,"Hdd1:\\SeriesDash360\\screenshots\\%04u%02u%02u_%02u%02u%02u.png",
    stime.wYear,stime.wMonth,stime.wDay,stime.wHour,stime.wMinute,stime.wSecond);
   ui.screenshot(path);
   captureRequested=false;
  }

  Sleep(16);
 }

 covers.stop();
 link.stop();
 ftp.stop();
 XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP,0);
 return 0;
}
