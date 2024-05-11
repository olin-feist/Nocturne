/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <wchar.h>
#include <fstream>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <chrono>
#include <thread>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include <opencv2/opencv.hpp>

using tfModelPtr = std::unique_ptr<tflite::FlatBufferModel>;

void configure_camera();
void print_camera_info(const v4l2_capability& capability);
int  open_device(const std::string& device);
int  get_frame(const int& fd);

class BoundingBox {
public:
    u_int32_t x;
    u_int32_t y;
    u_int32_t width;
    u_int32_t height;
    u_int32_t classId;  
    float conf;

    BoundingBox(u_int32_t,u_int32_t,u_int32_t,u_int32_t,u_int32_t,float);
    BoundingBox() = default;

    void print();
};