#pragma once
#include "sd360/Platform.hpp"

namespace sd360 {
class DesktopPlatform final : public IPlatform {
public:
    std::string platformName() const override;
    std::vector<std::filesystem::path> storageRoots() const override;
    SystemStats systemStats() const override;
    bool launchTitle(const std::filesystem::path& executable, std::string& error) override;
    bool rebootToSystemDashboard(std::string& error) override;
};
}
