#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace sd360 {

struct SystemStats {
    float cpuTempC = 0.0f;
    float gpuTempC = 0.0f;
    std::uint64_t freeBytes = 0;
    std::uint64_t totalBytes = 0;
    std::string ipAddress = "offline";
};

class IPlatform {
public:
    virtual ~IPlatform() = default;

    virtual std::string platformName() const = 0;
    virtual std::vector<std::filesystem::path> storageRoots() const = 0;
    virtual SystemStats systemStats() const = 0;

    // Launches a title through the already-authorized host environment.
    // The dashboard deliberately does not patch the kernel, hypervisor,
    // signatures or DRM. On Xbox 360 this is implemented by the user's
    // existing BadUpdate/FreeMyXe/XeUnshackle launch environment.
    virtual bool launchTitle(const std::filesystem::path& executable,
                             std::string& error) = 0;

    virtual bool rebootToSystemDashboard(std::string& error) = 0;
};

} // namespace sd360
