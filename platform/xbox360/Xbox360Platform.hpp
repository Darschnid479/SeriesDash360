#pragma once
#include "sd360/Platform.hpp"
namespace sd360 {
class Xbox360Platform final : public IPlatform {
public:
 std::string platformName() const override;
 std::vector<std::filesystem::path> storageRoots() const override;
 SystemStats systemStats() const override;
 bool launchTitle(const std::filesystem::path&,std::string&) override;
 bool rebootToSystemDashboard(std::string&) override;
};
}