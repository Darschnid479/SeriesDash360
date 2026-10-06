#include "sd360/GameLibrary.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace sd360 {
namespace {
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

Category guessCategory(const std::filesystem::path& p) {
    const std::string s = lower(p.string());
    if (s.find("emulator") != std::string::npos || s.find("emu") != std::string::npos) return Category::Emulators;
    if (s.find("homebrew") != std::string::npos) return Category::Homebrew;
    if (s.find("app") != std::string::npos || s.find("tool") != std::string::npos) return Category::Apps;
    return Category::Games;
}

bool isCandidateExecutable(const std::filesystem::path& p) {
    const auto name = lower(p.filename().string());
    return name == "default.xex" || name == "default.elf" || name == "default.xbe";
}
}

std::string makeStableId(const std::filesystem::path& executable) {
    const std::string normalized = lower(executable.lexically_normal().generic_string());
    const auto h = std::hash<std::string>{}(normalized);
    std::ostringstream os;
    os << std::hex << h;
    return os.str();
}

std::string prettifyTitle(const std::filesystem::path& folder) {
    std::string s = folder.filename().string();
    for (char& c : s) {
        if (c == '_' || c == '-') c = ' ';
    }
    if (s.empty()) s = "Untitled";
    bool cap = true;
    for (char& c : s) {
        if (std::isspace(static_cast<unsigned char>(c))) cap = true;
        else if (cap) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            cap = false;
        }
    }
    return s;
}

void GameLibrary::clear() { entries_.clear(); }

void GameLibrary::scan(const std::vector<std::filesystem::path>& roots, int maxDepth) {
    std::unordered_set<std::string> seen;
    std::vector<GameEntry> discovered;

    for (const auto& root : roots) {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec)) continue;

        std::filesystem::recursive_directory_iterator it(
            root,
            std::filesystem::directory_options::skip_permission_denied,
            ec);
        const std::filesystem::recursive_directory_iterator end;

        for (; !ec && it != end; it.increment(ec)) {
            if (it.depth() > maxDepth) {
                it.disable_recursion_pending();
                continue;
            }
            if (!it->is_regular_file(ec) || !isCandidateExecutable(it->path())) continue;

            GameEntry e;
            e.executable = it->path();
            e.root = it->path().parent_path();
            e.id = makeStableId(e.executable);
            e.title = prettifyTitle(e.root);
            e.category = guessCategory(e.root);

            for (const char* cover : {"cover.png", "cover.jpg", "icon.png", "folder.jpg"}) {
                auto candidate = e.root / cover;
                if (std::filesystem::exists(candidate, ec)) {
                    e.coverPath = candidate;
                    break;
                }
            }

            if (seen.insert(e.id).second) discovered.push_back(std::move(e));
        }
    }

    for (auto& e : discovered) {
        auto old = findById(e.id);
        if (old) {
            e.favorite = old->favorite;
            e.lastPlayedUnix = old->lastPlayedUnix;
            e.playCount = old->playCount;
        }
    }
    entries_ = std::move(discovered);
    std::sort(entries_.begin(), entries_.end(), [](const GameEntry& a, const GameEntry& b) {
        return lower(a.title) < lower(b.title);
    });
}

void GameLibrary::addOrUpdate(GameEntry entry) {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const GameEntry& e) { return e.id == entry.id; });
    if (it == entries_.end()) entries_.push_back(std::move(entry));
    else *it = std::move(entry);
}

std::vector<GameEntry> GameLibrary::search(const std::string& query) const {
    const std::string q = lower(query);
    std::vector<GameEntry> out;
    for (const auto& e : entries_) {
        if (q.empty() || lower(e.title).find(q) != std::string::npos) out.push_back(e);
    }
    return out;
}

std::vector<GameEntry> GameLibrary::favorites() const {
    std::vector<GameEntry> out;
    std::copy_if(entries_.begin(), entries_.end(), std::back_inserter(out), [](const GameEntry& e) { return e.favorite; });
    return out;
}

std::vector<GameEntry> GameLibrary::recent(std::size_t limit) const {
    std::vector<GameEntry> out = entries_;
    std::sort(out.begin(), out.end(), [](const GameEntry& a, const GameEntry& b) {
        return a.lastPlayedUnix > b.lastPlayedUnix;
    });
    out.erase(std::remove_if(out.begin(), out.end(), [](const GameEntry& e) { return e.lastPlayedUnix == 0; }), out.end());
    if (out.size() > limit) out.resize(limit);
    return out;
}

std::optional<GameEntry> GameLibrary::findById(const std::string& id) const {
    auto it = std::find_if(entries_.begin(), entries_.end(), [&](const GameEntry& e) { return e.id == id; });
    if (it == entries_.end()) return std::nullopt;
    return *it;
}

} // namespace sd360
