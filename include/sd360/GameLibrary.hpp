#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace sd360 {

enum class Category {
    Games,
    Homebrew,
    Emulators,
    Apps,
    Hidden
};

struct GameEntry {
    std::string id;
    std::string title;
    std::filesystem::path executable;
    std::filesystem::path root;
    std::filesystem::path coverPath;
    Category category = Category::Games;
    bool favorite = false;
    std::uint64_t lastPlayedUnix = 0;
    std::uint32_t playCount = 0;
};

class GameLibrary {
public:
    void clear();
    void scan(const std::vector<std::filesystem::path>& roots, int maxDepth = 4);
    void addOrUpdate(GameEntry entry);

    const std::vector<GameEntry>& entries() const { return entries_; }
    std::vector<GameEntry> search(const std::string& query) const;
    std::vector<GameEntry> favorites() const;
    std::vector<GameEntry> recent(std::size_t limit = 12) const;
    std::optional<GameEntry> findById(const std::string& id) const;

private:
    std::vector<GameEntry> entries_;
};

std::string makeStableId(const std::filesystem::path& executable);
std::string prettifyTitle(const std::filesystem::path& folder);

} // namespace sd360
