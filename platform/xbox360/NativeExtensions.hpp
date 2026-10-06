#pragma once
#include <xtl.h>
#include <string>
#include <vector>
#include "NativeRuntime.hpp"
#include "NativeState.hpp"

namespace sd360x {

struct PluginInfo {
 std::string path;
 std::string name;
 bool loaded;
 PluginInfo():loaded(false){}
};

class PluginManager {
public:
 void scan();
 bool load(size_t index);
 void loadAll();
 const std::vector<PluginInfo>& items() const { return items_; }
private:
 std::vector<PluginInfo> items_;
};

class ScriptRuntime {
public:
 bool runFile(const std::string& path,NativeRuntime& runtime,StateStore& state);
 void runStartupScripts(NativeRuntime& runtime,StateStore& state);
private:
 bool execute(const std::string& line,NativeRuntime& runtime,StateStore& state);
};

struct LinkPeer {
 std::string name;
 std::string ip;
 DWORD titleId;
 DWORD lastSeen;
 LinkPeer():titleId(0),lastSeen(0){}
};

class SystemLinkService {
public:
 SystemLinkService();
 ~SystemLinkService();
 bool start(unsigned short port=30720);
 void poll(DWORD titleId,const std::string& title);
 void stop();
 const std::vector<LinkPeer>& peers() const { return peers_; }
private:
 SOCKET socket_;
 unsigned short port_;
 DWORD lastBeacon_;
 std::vector<LinkPeer> peers_;
 void sendBeacon(DWORD titleId,const std::string& title);
 void receive();
};

class BackgroundCoverQueue {
public:
 BackgroundCoverQueue();
 ~BackgroundCoverQueue();
 bool start(NativeRuntime* runtime);
 void queue(size_t titleIndex);
 void stop();
private:
 NativeRuntime* runtime_;
 HANDLE thread_;
 HANDLE wake_;
 CRITICAL_SECTION lock_;
 std::vector<size_t> queue_;
 volatile bool running_;
 static DWORD WINAPI threadProc(LPVOID p);
 DWORD work();
};

}
