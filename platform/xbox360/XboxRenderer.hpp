#pragma once
namespace sd360 {
class XboxRenderer {
public:
 bool initialize();
 void beginFrame();
 void endFrame();
 void shutdown();
private: void* d3d_=nullptr; void* device_=nullptr;
};
}