#include "NativeRenderer.hpp"
namespace sd360x {
struct V { float x,y,z,rhw; DWORD c; float u,v; };
#define SD_FVF (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
NativeRenderer::NativeRenderer():d3d_(0),dev_(0),font_(0),background_(0xFF090B0D){}
NativeRenderer::~NativeRenderer(){if(font_)font_->Release();if(dev_)dev_->Release();if(d3d_)d3d_->Release();}
bool NativeRenderer::init(){
 d3d_=Direct3DCreate9(D3D_SDK_VERSION);if(!d3d_)return false;
 D3DPRESENT_PARAMETERS pp;ZeroMemory(&pp,sizeof(pp));pp.BackBufferWidth=1280;pp.BackBufferHeight=720;pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;pp.Windowed=FALSE;pp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
 if(FAILED(d3d_->CreateDevice(0,D3DDEVTYPE_HAL,0,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&dev_)))return false;
 D3DXCreateFontA(dev_,22,0,FW_NORMAL,1,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,"Arial",&font_);
 return true;
bool NativeRenderer::screenshot(const std::string& path){
 if(!dev_)return false;
 IDirect3DSurface9* surface=0;
 if(FAILED(dev_->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surface))||!surface)return false;
 HRESULT hr=D3DXSaveSurfaceToFileA(path.c_str(),D3DXIFF_PNG,surface,0,0);
 surface->Release();
 return SUCCEEDED(hr);
}
}

void NativeRenderer::begin(){dev_->Clear(0,0,D3DCLEAR_TARGET,background_,1.0f,0);dev_->BeginScene();}
void NativeRenderer::end(){dev_->EndScene();dev_->Present(0,0,0,0);}
void NativeRenderer::textured(float x,float y,float w,float h,IDirect3DTexture9* t,DWORD c){
 V v[4]={{x-.5f,y-.5f,0,1,c,0,0},{x+w-.5f,y-.5f,0,1,c,1,0},{x-.5f,y+h-.5f,0,1,c,0,1},{x+w-.5f,y+h-.5f,0,1,c,1,1}};
 dev_->SetTexture(0,t);dev_->SetFVF(SD_FVF);dev_->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);dev_->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);dev_->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);dev_->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,2,v,sizeof(V));dev_->SetTexture(0,0);
}
void NativeRenderer::rect(float x,float y,float w,float h,DWORD c){textured(x,y,w,h,0,c);}
void NativeRenderer::text(float x,float y,DWORD c,const std::string& s){if(!font_)return;RECT r={(LONG)x,(LONG)y,1260,710};font_->DrawTextA(0,s.c_str(),-1,&r,DT_LEFT|DT_NOCLIP,c);}
void NativeRenderer::cover(float x,float y,float w,float h,const std::string& p,DWORD fallback){
 IDirect3DTexture9* tex=0;if(SUCCEEDED(D3DXCreateTextureFromFileA(dev_,p.c_str(),&tex))&&tex){textured(x,y,w,h,tex,0xFFFFFFFF);tex->Release();}else rect(x,y,w,h,fallback);
}
}
