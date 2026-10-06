#include "Xbox360Platform.hpp"
#include "XboxInput.hpp"
#include "XboxRenderer.hpp"
#include <xtl.h>
int main(){
 sd360::Xbox360Platform platform; sd360::XboxRenderer renderer; sd360::XboxInput input;
 if(!renderer.initialize()) return 2;
 bool running=true;
 while(running){
   const auto pad=input.poll();
   if(pad.connected && (pad.buttons & XINPUT_GAMEPAD_BACK)) running=false;
   renderer.beginFrame();
   // Native Series-style tile renderer is layered here next.
   renderer.endFrame();
 }
 renderer.shutdown(); return 0;
}
}