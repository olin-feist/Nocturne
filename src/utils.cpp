#include <fcntl.h>
#include <iostream>
#include <string.h>
namespace utils{
    int open_device(const std::string& device){
        int fd =  open(device.c_str(), O_RDWR);
        if (fd == -1){
            std::cerr<<"Failed to open device: "<<device<<std::endl;
        }
        return fd;
    }

    void errno_exit(const std::string& s) {
        std::cerr<<s<<" Error "<< strerror(errno) <<std::endl;
        exit(EXIT_FAILURE);
    }
}
