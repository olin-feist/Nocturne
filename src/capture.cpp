#include "headers/capture.h"
#include "headers/utils.h"
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <linux/videodev2.h>

namespace nocturne
{
    int Capture::get_frame(char** buf, u_int32_t& size){
        if(fd == -1){return 1;};

        v4l2_buffer bufferinfo{0};
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = 0;


        // Dequeue
        if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) == -1){
            std::cerr<<"VIDIOC_DQBUF"<<std::endl;
            return 1;
        }

        buffer_size = bufferinfo.bytesused;
        //std::cout << "Buffer read: " << (double)bufferinfo.bytesused / 1024 << " KBytes of data" << std::endl;
        *buf = (char*) malloc(bufferinfo.bytesused*sizeof(char));
        memcpy(*buf,buffer,bufferinfo.bytesused);
        size = buffer_size;
        

        // Re-Que
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) == -1){
            std::cerr<<"Failed to Que Buffer, ioctl: VIDIOC_QBUF"<<std::endl;
            return 1;
        }

        return 0;
    }

    int Capture::configure_camera(){
        // Check capability
        v4l2_capability capability;
        if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
            std::cerr<<"Ioctl Error: VIDIOC_QUERYCAP"<<std::endl;
            return 1;
        }

        if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            std::cerr<<"No video capture device"<<std::endl;
            return 1;
        }

        if (!(capability.capabilities & V4L2_CAP_STREAMING)) {
            std::cerr<<"Does not support streaming i/o"<<std::endl;
            return 1;
        }
        // Set format
        v4l2_format image_fmt;
        image_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        image_fmt.fmt.pix.width = WIDTH;
        image_fmt.fmt.pix.height = HEIGHT;
        image_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        image_fmt.fmt.pix.field = V4L2_FIELD_ANY;
        if (-1 == ioctl(fd, VIDIOC_S_FMT, &image_fmt)) {
            std::cerr<<"Ioctl Error: VIDIOC_S_FMT"<<std::endl;
            return 1;
        }
        // Request buffers
        v4l2_requestbuffers requestBuffer = {0};
        requestBuffer.count = 1;
        requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        requestBuffer.memory = V4L2_MEMORY_MMAP;

        if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) == -1){
            std::cerr<<"Ioctl Error: VIDIOC_REQBUFS"<<std::endl;
            return 1;
        }

        // Get Buffer
        v4l2_buffer query_buf {0} ;
        query_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        query_buf.memory = V4L2_MEMORY_MMAP;
        query_buf.index = 0;
        
        if (ioctl(fd, VIDIOC_QUERYBUF, &query_buf)== -1) {
            std::cerr<<"Ioctl Error: VIDIOC_QUERYBUF"<<std::endl;
            return 1;
        }

        buffer = (char*) mmap(NULL, query_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, query_buf.m.offset);
        if(buffer == MAP_FAILED){
            std::cerr<<"Failed to map buffer"<<std::endl;
            return 1;
        }

        v4l2_buffer bufferinfo {0};
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = 0;
        // Que
        if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) == -1){
            std::cerr<<"Failed to Que Buffer, ioctl: VIDIOC_QBUF"<<std::endl;
            return 1;
        }
        // Stream On
        if(ioctl(fd, VIDIOC_STREAMON, &bufferinfo.type) == -1){
            std::cerr<<"Failed to to turn on stream, icotl: VIDIOC_STREAMON"<<std::endl;
            return 1;
        }
        return 0;
    }

    Capture::Capture(const std::string& device){
        fd = utils::open_device(device);
        if(fd == -1){
            throw std::runtime_error("Capture Constuct Failed: Bad FD");
        }
        if(configure_camera()==1){
            throw std::runtime_error("Failed to Configure Camera");
        }


    }
    Capture& Capture::operator=(Capture&& other){
        fd = other.fd;
        buffer = other.buffer;
        other.fd=-1;
        other.buffer = nullptr;
        return *this;
    }
    Capture::~Capture(){
        // Camera was never opened
        if(fd == -1){return;};
        
        v4l2_buffer bufferinfo{0};
        bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        bufferinfo.memory = V4L2_MEMORY_MMAP;
        bufferinfo.index = 0;
        // end streaming
        if(ioctl(fd, VIDIOC_STREAMOFF, &bufferinfo.type) < 0){
            std::cerr<<"Failed to end stream, icoctl: VIDIOC_STREAMOFF"<<std::endl;
        }

        // Unmap
        if(buffer != NULL){
            if (-1 == munmap(buffer, buffer_size)){
                utils::errno_exit("munmap");
            }
        }

        // Close device
        if(close(fd) == -1){
            std::cerr<<"Failed to open device fd: "<<fd<<std::endl;
            utils::errno_exit("Close");
        }
    }

} // nocturne
