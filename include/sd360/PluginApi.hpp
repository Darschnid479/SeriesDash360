#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <vector>
namespace sd360 {
struct PluginCommand { std::string name; std::string description; std::function<bool(std::string&)> run; };
class PluginRegistry {
public:
    bool add(PluginCommand command);
    const std::vector<PluginCommand>& commands() const { return commands_; }
private:
    std::vector<PluginCommand> commands_;
};
struct ScriptPolicy {
    std::filesystem::path root;
    bool allowNetwork=false;
    bool allowLaunch=false;
    bool isPathAllowed(const std::filesystem::path& p) const;
};
}