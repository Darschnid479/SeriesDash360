#include "sd360/Settings.hpp"

#include <fstream>
#include <sstream>

namespace sd360 {
namespace {
std::string trim(std::string s) {
    const char* ws = " \t\r\n";
    const auto a = s.find_first_not_of(ws);
    if (a == std::string::npos) return {};
    const auto b = s.find_last_not_of(ws);
    return s.substr(a, b - a + 1);
}

bool parseBool(const std::string& v) {
    return v == "1" || v == "true" || v == "yes" || v == "on";
}
}

bool Settings::load(const std::filesystem::path& file, std::string& error) {
    std::ifstream in(file);
    if (!in) {
        error = "Could not open settings file: " + file.string();
        return false;
    }

    scanPaths.clear();
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = trim(line.substr(0, eq));
        const std::string value = trim(line.substr(eq + 1));
        if (key == "theme") theme = value;
        else if (key == "reduce_motion") reduceMotion = parseBool(value);
        else if (key == "show_hidden") showHidden = parseBool(value);
        else if (key == "enable_ftp") enableFtp = parseBool(value);
        else if (key == "ftp_port") ftpPort = std::stoi(value);
        else if (key == "scan_path") scanPaths.emplace_back(value);
    }
    return true;
}

bool Settings::save(const std::filesystem::path& file, std::string& error) const {
    std::ofstream out(file, std::ios::trunc);
    if (!out) {
        error = "Could not write settings file: " + file.string();
        return false;
    }
    out << "theme=" << theme << "\n";
    out << "reduce_motion=" << (reduceMotion ? "true" : "false") << "\n";
    out << "show_hidden=" << (showHidden ? "true" : "false") << "\n";
    out << "enable_ftp=" << (enableFtp ? "true" : "false") << "\n";
    out << "ftp_port=" << ftpPort << "\n";
    for (const auto& p : scanPaths) out << "scan_path=" << p.string() << "\n";
    return true;
}

} // namespace sd360
