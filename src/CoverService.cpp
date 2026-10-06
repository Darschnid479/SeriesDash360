#include "sd360/CoverService.hpp"
#include <iomanip>
#include <sstream>
namespace sd360 {
std::filesystem::path CoverService::cachedPath(std::uint32_t id) const {
    std::ostringstream s; s<<std::uppercase<<std::hex<<std::setw(8)<<std::setfill('0')<<id;
    return cache_/(s.str()+".jpg");
}
bool CoverService::ensure(std::uint32_t id,const std::string& url,HttpDownloadFn dl,std::string& error) const {
    std::error_code ec; auto out=cachedPath(id);
    if(std::filesystem::exists(out,ec) && std::filesystem::file_size(out,ec)>0) return true;
    std::filesystem::create_directories(cache_,ec);
    if(ec){error="Unable to create cover cache";return false;}
    if(!dl){error="No HTTP download backend is installed";return false;}
    return dl(url,out,error);
}
}