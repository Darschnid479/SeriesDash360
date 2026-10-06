#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sd360 {

struct Settings {
    std::string theme = "series-dark";
    bool reduceMotion = false;
    bool showHidden = false;
    bool enableFtp = false;
    int ftpPort = 7564;
    std::vector<std::filesystem::path> scanPaths;

    bool load(const std::filesystem::path& file, std::string& error);
    bool save(const std::filesystem::path& file, std::string& error) const;
};

} // namespace sd360
