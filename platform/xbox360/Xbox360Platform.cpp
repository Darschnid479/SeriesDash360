#include "Xbox360Platform.hpp"
#ifdef _XBOX
#include <xtl.h>
#endif
namespace sd360 {
std::string Xbox360Platform::platformName() const { return "Xbox 360"; }
std::vector<std::filesystem::path> Xbox360Platform::storageRoots() const {
 return {"Hdd1:/Games","Hdd1:/Content","Usb0:/Games","Usb0:/Content","Usb1:/Games"};
}
SystemStats Xbox360Platform::systemStats() const {
 SystemStats s;
#ifdef _XBOX
 BYTE cmd[16]={0x07}, out[16]={0};
 HalSendSMCMessage(cmd,out);
 s.cpuTempC=static_cast<float>(out[1]); s.gpuTempC=static_cast<float>(out[2]);
#endif
 return s;
}
bool Xbox360Platform::launchTitle(const std::filesystem::path& p,std::string& error) {
#ifdef _XBOX
 const std::string x=p.string(); XLaunchNewImage(x.c_str(),0); error="XLaunchNewImage returned unexpectedly"; return false;
#else
 (void)p; error="Xbox 360 launch backend is only available in an Xbox build"; return false;
#endif
}
bool Xbox360Platform::rebootToSystemDashboard(std::string& error) {
#ifdef _XBOX
 XLaunchNewImage(XLAUNCH_KEYWORD_DEFAULT_APP,0); error="Dashboard launch returned unexpectedly"; return false;
#else
 error="Xbox 360 launch backend is only available in an Xbox build"; return false;
#endif
}
}