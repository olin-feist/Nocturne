#include <string>

namespace nocturne {
    class Capture{
        uint32_t WIDTH  = 640;
        uint32_t HEIGHT = 480;
        int fd{-1};
        char* buffer{NULL};
        u_int32_t buffer_size{0};
        int configure_camera();

    public:
        int get_frame(char**,u_int32_t&);
        
        Capture(const std::string&);
        Capture& operator=(Capture&&);
        Capture() = default;
        ~Capture();
    };
}