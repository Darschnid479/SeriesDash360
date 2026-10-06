#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
namespace sd360 {
struct XexMetadata {
    std::uint32_t titleId = 0;
    std::uint32_t mediaId = 0;
    std::uint32_t version = 0;
    std::uint32_t baseVersion = 0;
};
std::optional<XexMetadata> readXexMetadata(const std::filesystem::path& path);
}