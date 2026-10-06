#include "sd360/App.hpp"

#include <chrono>

namespace sd360 {

App::App(std::unique_ptr<IPlatform> platform) : platform_(std::move(platform)) {}

bool App::initialize(const std::filesystem::path& settingsFile, std::string& error) {
    settingsFile_ = settingsFile;
    if (std::filesystem::exists(settingsFile)) {
        if (!settings_.load(settingsFile, error)) return false;
    }
    if (settings_.scanPaths.empty()) settings_.scanPaths = platform_->storageRoots();
    rescan();
    return true;
}

void App::rescan() {
    library_.scan(settings_.scanPaths);
}

bool App::launch(const std::string& id, std::string& error) {
    auto entry = library_.findById(id);
    if (!entry) {
        error = "Title not found";
        return false;
    }
    if (!platform_->launchTitle(entry->executable, error)) return false;

    auto updated = *entry;
    updated.playCount++;
    updated.lastPlayedUnix = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    library_.addOrUpdate(std::move(updated));
    return true;
}

} // namespace sd360
