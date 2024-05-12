#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <linux/videodev2.h>

#include "headers/utils.h"

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
    void save_jpeg(const std::string& s,const char* buf,const u_int32_t& size){
        std::ofstream outFile;
        outFile.open(s, std::ios::binary | std::ios::trunc);
        outFile.write(buf, size);
        outFile.close();
    }
    void print_camera_info(const v4l2_capability& capability){
        std::cout<<"Driver: "<<capability.driver<<std::endl;
        std::cout<<"Card: "<<capability.card<<std::endl;
        std::cout<<"Bus info: "<<capability.bus_info<<std::endl;
        std::cout<<"Version: "<<capability.version<<std::endl;
        std::cout<<"Capabilities: "<<capability.capabilities<<std::endl;
        std::cout<<"Device Caps: "<<capability.device_caps<<std::endl;
    }  

    std::vector<std::string> get_class_names(const std::string& path){
        std::ifstream in(path);
        std::string line;
        std::vector<std::string> v;
        
        while(std::getline(in,line)){
            v.push_back(line);
        }

        return v;
    }
}
