#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace sd360 {

struct FileItem {
    std::filesystem::path path;
    bool isDirectory = false;
    std::uint64_t size = 0;
};

class FileManager {
public:
    std::vector<FileItem> list(const std::filesystem::path& folder, std::string& error) const;
    bool copy(const std::filesystem::path& from, const std::filesystem::path& to, std::string& error) const;
    bool move(const std::filesystem::path& from, const std::filesystem::path& to, std::string& error) const;
    bool remove(const std::filesystem::path& target, std::string& error) const;
    bool createDirectory(const std::filesystem::path& folder, std::string& error) const;
};

} // namespace sd360
