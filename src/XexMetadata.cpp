#include "sd360/XexMetadata.hpp"
#include <array>
#include <fstream>
namespace sd360 {
namespace {
std::uint32_t be32(const unsigned char* p) {
    return (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) |
           (std::uint32_t(p[2]) << 8) | std::uint32_t(p[3]);
}
bool readAt(std::ifstream& f, std::uint64_t off, unsigned char* p, std::size_t n) {
    f.seekg(static_cast<std::streamoff>(off), std::ios::beg);
    return bool(f.read(reinterpret_cast<char*>(p), static_cast<std::streamsize>(n)));
}
}
std::optional<XexMetadata> readXexMetadata(const std::filesystem::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return std::nullopt;
    std::array<unsigned char, 24> h{};
    if (!readAt(f, 0, h.data(), h.size()) || be32(h.data()) != 0x58455832u) return std::nullopt;
    const std::uint32_t count = be32(h.data() + 20);
    if (count > 4096) return std::nullopt;
    for (std::uint32_t i=0; i<count; ++i) {
        std::array<unsigned char, 8> oh{};
        if (!readAt(f, 24ull + i*8ull, oh.data(), oh.size())) return std::nullopt;
        if (be32(oh.data()) != 0x00040006u) continue; // XEX_HEADER_EXECUTION_ID
        const std::uint32_t off = be32(oh.data()+4);
        std::array<unsigned char, 24> x{};
        if (!readAt(f, off, x.data(), x.size())) return std::nullopt;
        XexMetadata m;
        m.mediaId = be32(x.data());
        m.version = be32(x.data()+4);
        m.baseVersion = be32(x.data()+8);
        m.titleId = be32(x.data()+12);
        return m;
    }
    return std::nullopt;
}
}