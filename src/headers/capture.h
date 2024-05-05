#include <string>

namespace nocturne {
    class Capture{
        const uint32_t WIDTH  = 320;
        const uint32_t HEIGHT = 320;
        int fd;
        char* buffer{NULL};
        u_int32_t buffer_size{0};
        int configure_camera();

    public:
        int get_frame(char**,u_int32_t&);
        
        Capture(const std::string&);
        ~Capture();
    };
} // nocturne