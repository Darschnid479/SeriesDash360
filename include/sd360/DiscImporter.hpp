#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
namespace sd360 {
struct CopyProgress { std::uint64_t copied=0, total=0; std::filesystem::path current; };
using CopyProgressFn = std::function<void(const CopyProgress&)>;
bool copyMountedDisc(const std::filesystem::path& discRoot,
                     const std::filesystem::path& destination,
                     CopyProgressFn progress, std::string& error);
}