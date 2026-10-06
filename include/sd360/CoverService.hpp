#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
namespace sd360 {
using HttpDownloadFn=std::function<bool(const std::string&,const std::filesystem::path&,std::string&)>;
class CoverService {
public:
    explicit CoverService(std::filesystem::path cache):cache_(std::move(cache)){}
    std::filesystem::path cachedPath(std::uint32_t titleId) const;
    bool ensure(std::uint32_t titleId,const std::string& url,HttpDownloadFn download,std::string& error) const;
private: std::filesystem::path cache_;
};
}