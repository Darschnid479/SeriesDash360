#include "XboxRenderer.hpp"
#ifdef _XBOX
#include <xtl.h>
#endif
namespace sd360 {
bool XboxRenderer::initialize() {
#ifdef _XBOX
 auto* d3d=Direct3DCreate9(D3D_SDK_VERSION); if(!d3d)return false;
 D3DPRESENT_PARAMETERS pp={}; pp.BackBufferWidth=1280; pp.BackBufferHeight=720; pp.BackBufferFormat=D3DFMT_A8R8G8B8;
 pp.BackBufferCount=1; pp.MultiSampleType=D3DMULTISAMPLE_NONE; pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
 pp.hDeviceWindow=nullptr; pp.Windowed=FALSE; pp.EnableAutoDepthStencil=FALSE; pp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
 IDirect3DDevice9* dev=nullptr;
 if(FAILED(d3d->CreateDevice(0,D3DDEVTYPE_HAL,nullptr,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&dev))){d3d->Release();return false;}
 d3d_=d3d; device_=dev; return true;
#else
 return false;
#endif
}
void XboxRenderer::beginFrame(){
#ifdef _XBOX
 auto* d=static_cast<IDirect3DDevice9*>(device_); if(d){d->Clear(0,nullptr,D3DCLEAR_TARGET,0xFF101216,1.0f,0);d->BeginScene();}
#endif
}
void XboxRenderer::endFrame(){
#ifdef _XBOX
 auto* d=static_cast<IDirect3DDevice9*>(device_); if(d){d->EndScene();d->Present(nullptr,nullptr,nullptr,nullptr);}
#endif
}
void XboxRenderer::shutdown(){
#ifdef _XBOX
 if(device_){static_cast<IDirect3DDevice9*>(device_)->Release();device_=nullptr;}
 if(d3d_){static_cast<IDirect3D9*>(d3d_)->Release();d3d_=nullptr;}
#endif
}
}