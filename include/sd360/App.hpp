#pragma once

#include "sd360/GameLibrary.hpp"
#include "sd360/Platform.hpp"
#include "sd360/Settings.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace sd360 {

class App {
public:
    explicit App(std::unique_ptr<IPlatform> platform);

    bool initialize(const std::filesystem::path& settingsFile, std::string& error);
    void rescan();
    bool launch(const std::string& id, std::string& error);

    GameLibrary& library() { return library_; }
    const GameLibrary& library() const { return library_; }
    Settings& settings() { return settings_; }
    const Settings& settings() const { return settings_; }
    IPlatform& platform() { return *platform_; }

private:
    std::unique_ptr<IPlatform> platform_;
    Settings settings_;
    GameLibrary library_;
    std::filesystem::path settingsFile_;
};

} // namespace sd360
