#include "sd360/DiscImporter.hpp"
#include <fstream>
#include <vector>
namespace sd360 {
bool copyMountedDisc(const std::filesystem::path& src, const std::filesystem::path& dst,
                     CopyProgressFn cb, std::string& error) {
    std::error_code ec;
    if (!std::filesystem::exists(src, ec) || !std::filesystem::is_directory(src, ec)) {
        error="Disc root is not mounted"; return false;
    }
    std::uint64_t total=0, copied=0;
    for (auto it=std::filesystem::recursive_directory_iterator(src, std::filesystem::directory_options::skip_permission_denied, ec);
         !ec && it!=std::filesystem::recursive_directory_iterator(); it.increment(ec))
        if (it->is_regular_file(ec)) total += it->file_size(ec);
    if (ec) { error="Unable to enumerate mounted disc"; return false; }
    std::filesystem::create_directories(dst, ec);
    std::vector<char> buf(1024*1024);
    for (auto it=std::filesystem::recursive_directory_iterator(src, std::filesystem::directory_options::skip_permission_denied, ec);
         !ec && it!=std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        auto rel=std::filesystem::relative(it->path(), src, ec); if(ec) break;
        auto out=dst/rel;
        if (it->is_directory(ec)) { std::filesystem::create_directories(out, ec); continue; }
        if (!it->is_regular_file(ec)) continue;
        std::filesystem::create_directories(out.parent_path(), ec);
        std::ifstream in(it->path(),std::ios::binary); std::ofstream o(out,std::ios::binary|std::ios::trunc);
        if(!in||!o){error="Unable to open file during disc copy: "+it->path().string();return false;}
        while(in){ in.read(buf.data(),buf.size()); auto n=in.gcount(); if(n<=0)break; o.write(buf.data(),n);
            if(!o){error="Write failed while copying disc";return false;} copied+=std::uint64_t(n);
            if(cb)cb({copied,total,it->path()}); }
    }
    if(ec){error="Disc copy enumeration failed";return false;}
    error.clear(); return true;
}
}