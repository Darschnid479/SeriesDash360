#include "XboxInput.hpp"
#ifdef _XBOX
#include <xtl.h>
#endif
namespace sd360 {
PadState XboxInput::poll(unsigned index) const {
 PadState o;
#ifdef _XBOX
 XINPUT_STATE s={}; if(XInputGetState(index,&s)!=ERROR_SUCCESS)return o; o.connected=true;
 o.buttons=s.Gamepad.wButtons; o.lx=s.Gamepad.sThumbLX/32767.0f; o.ly=s.Gamepad.sThumbLY/32767.0f;
 o.rx=s.Gamepad.sThumbRX/32767.0f; o.ry=s.Gamepad.sThumbRY/32767.0f;
 o.lt=s.Gamepad.bLeftTrigger/255.0f; o.rt=s.Gamepad.bRightTrigger/255.0f;
#else
 (void)index;
#endif
 return o;
}
}