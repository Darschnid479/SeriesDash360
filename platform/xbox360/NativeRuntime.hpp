#pragma once
#include <string>
#include <vector>
#include <xtl.h>

namespace sd360x {
struct TitleEntry {
  std::string title;
  std::string xex;
  DWORD titleId;
  std::string cover;
  bool favorite;
  TitleEntry():titleId(0),favorite(false){}
};
struct SystemInfo { int cpu,gpu,edram,mb; std::string ip; SystemInfo():cpu(0),gpu(0),edram(0),mb(0),ip("offline"){} };

class NativeRuntime {
public:
  NativeRuntime();
  void scan();
  const std::vector<TitleEntry>& titles() const { return titles_; }
  bool launch(size_t index);
  SystemInfo systemInfo() const;
  bool importDisc(const char* destination);
  bool refreshCover(size_t index);
  void setCoverBase(const std::string& base){coverBase_=base;}
private:
  void scanRoot(const char* root);
  void scanFolder(const std::string& folder,int depth);
  DWORD readTitleId(const char* path) const;
  bool copyTree(const std::string& src,const std::string& dst);
  std::vector<TitleEntry> titles_;
  std::string coverBase_;
};
}
