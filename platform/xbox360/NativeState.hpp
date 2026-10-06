#pragma once
#include <xtl.h>
#include <map>
#include <string>
#include <vector>
#include "NativeRuntime.hpp"

namespace sd360x {

enum LibraryFilter {
 FILTER_ALL=0,
 FILTER_GAMES,
 FILTER_HOMEBREW,
 FILTER_FAVORITES,
 FILTER_RECENT
};

struct TitleState {
 bool favorite;
 ULONGLONG lastPlayed;
 std::string category;
 std::string customTitle;
 TitleState():favorite(false),lastPlayed(0),category("Games"){}
};

struct Theme {
 DWORD background;
 DWORD panel;
 DWORD accent;
 DWORD text;
 DWORD muted;
 Theme():background(0xFF090B0D),panel(0xFF15181B),accent(0xFF107C10),text(0xFFFFFFFF),muted(0xFFB5BABE){}
};

class StateStore {
public:
 StateStore();
 void load();
 void save() const;
 TitleState state(DWORD titleId) const;
 void toggleFavorite(DWORD titleId);
 void recordPlayed(DWORD titleId);
 void setCategory(DWORD titleId,const std::string& category);
 void setCustomTitle(DWORD titleId,const std::string& title);
 void apply(std::vector<TitleEntry>& titles) const;
 std::vector<size_t> filter(const std::vector<TitleEntry>& titles,const std::string& query,LibraryFilter filter) const;
 const Theme& theme() const { return theme_; }
 const std::string& themeName() const { return themeName_; }
 void setTheme(const std::string& name);
private:
 std::map<DWORD,TitleState> states_;
 Theme theme_;
 std::string themeName_;
 std::string path_;
};

bool showKeyboard(const wchar_t* title,const wchar_t* description,std::string& result);

}
