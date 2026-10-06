#pragma once
#include <xtl.h>
#include <d3dx9.h>
#include <string>

namespace sd360x {

class NativeRenderer {
public:
 NativeRenderer();
 ~NativeRenderer();
 bool init();
 void begin();
 void end();
 void rect(float x,float y,float w,float h,DWORD color);
 void text(float x,float y,DWORD color,const std::string& value);
 void cover(float x,float y,float w,float h,const std::string& path,DWORD fallback);
 void setBackground(DWORD color){background_=color;}
 bool screenshot(const std::string& path);
private:
 IDirect3D9* d3d_;
 IDirect3DDevice9* dev_;
 ID3DXFont* font_;
 DWORD background_;
 void textured(float x,float y,float w,float h,IDirect3DTexture9* tex,DWORD color);
};

}
