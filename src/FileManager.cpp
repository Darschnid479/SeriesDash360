#include "sd360/FileManager.hpp"

#include <algorithm>

namespace sd360 {

std::vector<FileItem> FileManager::list(const std::filesystem::path& folder, std::string& error) const {
    std::vector<FileItem> out;
    std::error_code ec;
    for (const auto& e : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied, ec)) {
        FileItem item;
        item.path = e.path();
        item.isDirectory = e.is_directory(ec);
        item.size = item.isDirectory ? 0 : static_cast<std::uint64_t>(e.file_size(ec));
        out.push_back(std::move(item));
    }
    if (ec) error = ec.message();
    std::sort(out.begin(), out.end(), [](const FileItem& a, const FileItem& b) {
        if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
        return a.path.filename().string() < b.path.filename().string();
    });
    return out;
}

bool FileManager::copy(const std::filesystem::path& from, const std::filesystem::path& to, std::string& error) const {
    std::error_code ec;
    if (std::filesystem::is_directory(from, ec)) {
        std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    } else {
        std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing, ec);
    }
    if (ec) { error = ec.message(); return false; }
    return true;
}

bool FileManager::move(const std::filesystem::path& from, const std::filesystem::path& to, std::string& error) const {
    std::error_code ec;
    std::filesystem::rename(from, to, ec);
    if (ec) { error = ec.message(); return false; }
    return true;
}

bool FileManager::remove(const std::filesystem::path& target, std::string& error) const {
    std::error_code ec;
    std::filesystem::remove_all(target, ec);
    if (ec) { error = ec.message(); return false; }
    return true;
}

bool FileManager::createDirectory(const std::filesystem::path& folder, std::string& error) const {
    std::error_code ec;
    std::filesystem::create_directories(folder, ec);
    if (ec) { error = ec.message(); return false; }
    return true;
}

} // namespace sd360
