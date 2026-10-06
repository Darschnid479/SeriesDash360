#pragma once
#include <string>
namespace sd360x {
bool networkStart();
bool httpGetText(const std::string& url,std::string& out);
bool httpDownloadFile(const std::string& url,const std::string& path);
class FtpServer {
public: FtpServer(); ~FtpServer(); bool start(unsigned short port=7564); void poll(); void stop();
private:
 int listen_,client_,dataListen_,data_;
 std::string cwd_;
 void command(const char* line);
 bool openPassive();
 bool acceptData();
 void closeData();
 std::string resolve(const std::string& p) const;
 void sendLine(const std::string& s);
 void listPath(const std::string& p);
 void sendFile(const std::string& p);
 void receiveFile(const std::string& p);
};
}
