#include <string>
namespace utils{
    int open_device(const std::string&);
    void errno_exit(const std::string&);
    void save_jpeg(const std::string&,const char*,const u_int32_t&);
    void print_camera_info(const struct v4l2_capability&);
}
