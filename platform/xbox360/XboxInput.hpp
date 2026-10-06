#pragma once
#include <cstdint>
namespace sd360 {
struct PadState { bool connected=false; std::uint16_t buttons=0; float lx=0,ly=0,rx=0,ry=0,lt=0,rt=0; };
class XboxInput { public: PadState poll(unsigned index=0) const; };
}