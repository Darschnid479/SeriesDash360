#pragma once
#include <xtl.h>
#include <string>
#include <vector>

namespace sd360x {

struct FileEntry {
 std::string name;
 std::string path;
 bool directory;
 ULONGLONG size;
 FileEntry():directory(false),size(0){}
};

class NativeFileManager {
public:
 NativeFileManager();
 const std::string& path() const { return path_; }
 const std::vector<FileEntry>& items() const { return items_; }
 void setPath(const std::string& p);
 bool up();
 void refresh();
 bool enter(size_t i);
 bool remove(size_t i);
 bool createFolder(const std::string& name);
 bool copy(size_t i,const std::string& destination);
 bool move(size_t i,const std::string& destination);
private:
 std::string path_;
 std::vector<FileEntry> items_;
 bool copyTree(const std::string& src,const std::string& dst);
 bool removeTree(const std::string& path);
};

struct TitleUpdate {
 std::string path;
 std::string name;
 bool enabled;
 TitleUpdate():enabled(true){}
};

class TitleUpdateManager {
public:
 void scan(DWORD titleId);
 const std::vector<TitleUpdate>& items() const { return items_; }
 bool setEnabled(size_t index,bool enabled);
private:
 std::vector<TitleUpdate> items_;
 void scanFolder(const std::string& path);
};

struct SaveEntry {
 std::string profile;
 std::string path;
 std::string name;
 ULONGLONG size;
 SaveEntry():size(0){}
};

class SaveBrowser {
public:
 void scan(DWORD titleId);
 const std::vector<SaveEntry>& items() const { return items_; }
private:
 std::vector<SaveEntry> items_;
 void scanTree(const std::string& path,const std::string& profile,int depth);
};

struct AchievementEntry {
 DWORD id;
 DWORD score;
 DWORD flags;
 std::string label;
 std::string description;
 AchievementEntry():id(0),score(0),flags(0){}
};

class AchievementBrowser {
public:
 bool load(DWORD titleId,DWORD userIndex=0);
 const std::vector<AchievementEntry>& items() const { return items_; }
private:
 std::vector<AchievementEntry> items_;
};

}
