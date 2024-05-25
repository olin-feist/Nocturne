/*-------------------------------------------
                Includes
-------------------------------------------*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <linux/videodev2.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include "capture.h"


void configure_camera();
void print_camera_info(const v4l2_capability& capability);
int  open_device(const std::string& device);
int  get_frame(const int& fd);

class BoundingBox {
public:
    float x;
    float y;
    float width;
    float height;
    u_int32_t classId;  
    float obj_conf;
    float class_conf;

    BoundingBox(u_int32_t,u_int32_t,u_int32_t,u_int32_t,u_int32_t,float,float);
    BoundingBox() = default;

    void print();
};

class ObjectDetection {
    using tfModelPtr = std::unique_ptr<tflite::FlatBufferModel>;
    using tfInterPtr = std::unique_ptr<tflite::Interpreter>;

    tfModelPtr model;
    tfInterPtr interpreter;
    float* input_tensor{NULL};
    float* output_tensor{NULL};
    u_int32_t model_image_width{0};
    u_int32_t model_image_height{0};
    u_int32_t model_image_chnls{0};
    u_int32_t model_num_classes{0};
    u_int32_t model_num_canidates{0};

    nocturne::Capture cam1;
    char* buf{NULL};
    u_int32_t size{0};

    std::vector<BoundingBox> get_boxes();
public:
    ObjectDetection(const std::string&);
    ObjectDetection();
    ObjectDetection& operator=(ObjectDetection&&);
    ~ObjectDetection();
    std::vector<BoundingBox> detect();

    const u_int32_t& getModelWidth()  const {return  model_image_width;}
    const u_int32_t& getModelHeight() const {return model_image_height;}
};