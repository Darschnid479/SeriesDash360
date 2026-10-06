#include "sd360/PluginApi.hpp"
#include <algorithm>
namespace sd360 {
bool PluginRegistry::add(PluginCommand c) {
    if(c.name.empty() || !c.run) return false;
    if(std::any_of(commands_.begin(),commands_.end(),[&](const auto& x){return x.name==c.name;})) return false;
    commands_.push_back(std::move(c)); return true;
}
bool ScriptPolicy::isPathAllowed(const std::filesystem::path& p) const {
    std::error_code ec;
    auto base=std::filesystem::weakly_canonical(root,ec); if(ec)return false;
    auto candidate=std::filesystem::weakly_canonical(p,ec); if(ec)return false;
    auto b=base.generic_string(), c=candidate.generic_string();
    return c==b || (c.size()>b.size() && c.compare(0,b.size(),b)==0 && c[b.size()]=='/');
}
}