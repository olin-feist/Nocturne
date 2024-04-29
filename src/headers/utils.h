#include <string>
namespace utils{
    int open_device(const std::string& device);
    void errno_exit(const std::string& s);
}
