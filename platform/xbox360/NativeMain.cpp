#include "NativeRuntime.hpp"
#include "NativeRenderer.hpp"
#include "NativeNetwork.hpp"
#include <xtl.h>
#include <cstdio>

using namespace sd360x;
static bool pressed(WORD now,WORD old,WORD key){return (now&key)&&!(old&key);}
int main(){
 NativeRenderer ui;if(!ui.init())return 2;
 networkStart();FtpServer ftp;ftp.start(7564);
 NativeRuntime rt;rt.scan();
 size_t selected=0;WORD oldButtons=0;bool running=true;bool systemPage=false;
 while(running){
  ftp.poll();
  XINPUT_STATE st;ZeroMemory(&st,sizeof(st));WORD b=0;if(XInputGetState(0,&st)==ERROR_SUCCESS)b=st.Gamepad.wButtons;
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_RIGHT)&&selected+1<rt.titles().size())selected++;
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_DPAD_LEFT)&&selected>0)selected--;
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_A)&&!systemPage)rt.launch(selected);
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_Y)&&!systemPage)rt.refreshCover(selected);
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_X)&&!systemPage)rt.importDisc("Hdd1:\\Games\\DiscImport");
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_START))systemPage=!systemPage;
  if(pressed(b,oldButtons,XINPUT_GAMEPAD_BACK))running=false;
  oldButtons=b;

  ui.begin();
  ui.text(34,24,0xFFFFFFFF,"SERIESDASH360  1.0");
  ui.text(1020,24,0xFFB5BABE,"A Launch  Y Cover  X Copy Disc");
  if(systemPage){
    SystemInfo s=rt.systemInfo();char line[128];
    ui.text(34,92,0xFFFFFFFF,"System");
    ui.rect(34,135,280,150,0xFF15181B);sprintf(line,"CPU  %d C",s.cpu);ui.text(54,170,0xFFFFFFFF,line);
    ui.rect(330,135,280,150,0xFF15181B);sprintf(line,"GPU  %d C",s.gpu);ui.text(350,170,0xFFFFFFFF,line);
    ui.rect(626,135,280,150,0xFF15181B);sprintf(line,"EDRAM  %d C",s.edram);ui.text(646,170,0xFFFFFFFF,line);
    ui.rect(922,135,280,150,0xFF15181B);sprintf(line,"BOARD  %d C",s.mb);ui.text(942,170,0xFFFFFFFF,line);
    ui.text(34,340,0xFFB5BABE,"START: library   BACK: exit to host");
  }else{
    ui.text(34,78,0xFFFFFFFF,"My games");
    const std::vector<TitleEntry>& ts=rt.titles();
    const size_t first=(selected/6)*6;
    for(size_t i=0;i<6;i++){size_t n=first+i;if(n>=ts.size())break;float x=34.0f+i*202.0f;DWORD frame=(n==selected)?0xFFFFFFFF:0xFF15181B;
      ui.rect(x-4,124,196,310,frame);ui.cover(x,128,188,250,ts[n].cover,0xFF1C4028);ui.rect(x,378,188,52,0xE8000000);ui.text(x+8,390,0xFFFFFFFF,ts[n].title);
    }
    char count[64];sprintf(count,"%u titles",(unsigned)ts.size());ui.text(34,466,0xFFB5BABE,count);
    ui.rect(34,510,286,120,0xFF15181B);ui.text(54,535,0xFFFFFFFF,"File manager");ui.text(54,568,0xFFB5BABE,"HDD / USB content");
    ui.rect(334,510,286,120,0xFF15181B);ui.text(354,535,0xFFFFFFFF,"Disc to HDD");ui.text(354,568,0xFFB5BABE,"Press X to import");
    ui.rect(634,510,286,120,0xFF15181B);ui.text(654,535,0xFFFFFFFF,"Cover art");ui.text(654,568,0xFFB5BABE,"Press Y to download");
    ui.rect(934,510,268,120,0xFF15181B);ui.text(954,535,0xFFFFFFFF,"System");ui.text(954,568,0xFFB5BABE,"Press START");
  }
  ui.end();Sleep(16);
 }
 ftp.stop();XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP,0);return 0;
}
