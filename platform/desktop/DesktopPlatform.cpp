#include "DesktopPlatform.hpp"

#include <cstdlib>

namespace sd360 {
std::string DesktopPlatform::platformName() const { return "Desktop development host"; }

std::vector<std::filesystem::path> DesktopPlatform::storageRoots() const {
    return {std::filesystem::current_path() / "sample_library"};
}

SystemStats DesktopPlatform::systemStats() const {
    SystemStats s;
    std::error_code ec;
    auto sp = std::filesystem::space(std::filesystem::current_path(), ec);
    if (!ec) { s.freeBytes = sp.available; s.totalBytes = sp.capacity; }
    s.ipAddress = "127.0.0.1";
    return s;
}

bool DesktopPlatform::launchTitle(const std::filesystem::path& executable, std::string& error) {
    // Safety: desktop test build never tries to execute XEX/ELF console binaries.
    error = "Desktop preview: launch requested for " + executable.string();
    return false;
}

bool DesktopPlatform::rebootToSystemDashboard(std::string& error) {
    error = "Desktop preview does not provide a console reboot action.";
    return false;
}
}
