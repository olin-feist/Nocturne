#include "headers/detect.h"
#include "headers/utils.h"


#include <iostream>
#include <fstream>
#include <vector>

#include <opencv2/opencv.hpp>

namespace nocturne{

BoundingBox::BoundingBox(u_int32_t x_,u_int32_t y_,u_int32_t width_,u_int32_t height_,u_int32_t id_,float obj_conf_,float class_conf_){
    x=x_;
    y=y_;
    width=width_;
    height=height_;
    classId=id_;
    obj_conf=obj_conf_;
    class_conf=class_conf_;
}

void BoundingBox::print(){
    std::cout<<"X:\t "<<x<<std::endl;
    std::cout<<"Y:\t "<<y<<std::endl;
    std::cout<<"Width:\t "<<width<<std::endl;
    std::cout<<"Height:\t "<<height<<std::endl;
    std::cout<<"Obj Conf:\t "<<obj_conf<<std::endl;
    std::cout<<"Class Conf:\t "<<class_conf<<std::endl;
    std::cout<<"Class ID:\t "<<classId<<std::endl;
}

ObjectDetection::ObjectDetection(){}

ObjectDetection::ObjectDetection(const std::string& model_path){

    // Try all possible cameras
    int i;
    for(i=0;i<4;i++){
        try{
            cam1=Capture("/dev/video"+std::to_string(i));
            break;
        } catch(const std::exception& e){
            continue;
        }
    }
    if(i==4){throw std::runtime_error("Failed to Setup Camera");}

    // Init Model
    model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if(!model){
            throw std::runtime_error("Failed to Load model");
    }
    // Build Interpreter
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    if(builder(&interpreter)!=kTfLiteOk){
        throw std::runtime_error("Failed to build Interpreter");
    }

    interpreter->AllocateTensors();
    TfLiteTensor* intputtensor = interpreter->input_tensor(0);
    model_image_width = intputtensor->dims->data[1];
    model_image_height = intputtensor->dims->data[2];
    model_image_chnls = intputtensor->dims->data[3];
    TfLiteTensor* outputtensor = interpreter->output_tensor(0);
    model_num_canidates = outputtensor->dims->data[1];
    model_num_classes = outputtensor->dims->data[2];
    setup_gpu_compute();
}

ObjectDetection& ObjectDetection::operator=(ObjectDetection&& other){
    model=std::move(other.model);
    interpreter = std::move(other.interpreter);
    model_image_height = other.model_image_height;
    model_image_width= other.model_image_width;
    model_image_chnls= other.model_image_chnls;
    model_num_classes= other.model_num_classes;
    model_num_canidates= other.model_num_canidates;
    cam1=std::move(other.cam1);
    program=other.program;
    queue=other.queue;
    context=other.context;
    return *this;
}

ObjectDetection::~ObjectDetection(){
    if(buf!=NULL)
        free(buf);
}

std::vector<BoundingBox> ObjectDetection::detect(){
    if(model.get()==nullptr) return{};

    cam1.get_frame(&buf,size);
    cv::Mat rawData(1,size,CV_8SC1,(void*)buf);
    cv::Mat frame= cv::imdecode(rawData,cv::IMREAD_COLOR);
    cl::Image2D readImage(context,CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,cl::ImageFormat(CL_RGB, CL_SNORM_INT8),640,480,0,frame.data);
    gpu_resize_image(readImage);
    return{};
    cv::resize(frame, frame, cv::Size(model_image_width, model_image_height), 0, 0, cv::INTER_AREA);
    input_tensor = interpreter->typed_input_tensor<float>(0);

    // Copy float image into input tensor
    cv::Mat fimage;
    frame.convertTo(fimage, CV_32FC3,1.0/255.0);
    cv::cvtColor(fimage, fimage, cv::COLOR_BGR2RGB);
    memcpy(input_tensor, fimage.data,sizeof(float) * model_image_width * model_image_height * model_image_chnls);
    if (interpreter->Invoke() != kTfLiteOk) {
        std::cerr<<"Error invoking"<<std::endl;
        return {};
    }
    output_tensor = interpreter->typed_output_tensor<float>(0);
    return get_boxes();
}

std::vector<BoundingBox> ObjectDetection::get_boxes(){
        std::vector<BoundingBox> boxes;

        std::vector<int> clIds;
        std::vector<float> obj_conf;
        std::vector<float> class_conf;
        std::vector<cv::Rect> rect;

        float objConf;
        u_int32_t x,y,width,height,classId;
        for(int i=0;i<model_num_canidates*model_num_classes;i+=model_num_classes){
            // Box
            objConf = output_tensor[i+4];
            if(objConf<0.4)
                continue;
            
            width     = output_tensor[i + 2]*model_image_width;
            height    = output_tensor[i + 3]*model_image_height;
            /*if(width/model_image_width>=0.95&&height/model_image_height>=0){
                continue;
            }*/
            x = (output_tensor[i] * model_image_width )-width /2;
            y = (output_tensor[i + 1] * model_image_height)-height /2;

            // Class
            float max=-1.0;
            for(int j=i+5;j<i+80;j++){
                    if(output_tensor[j]>max){
                        max=output_tensor[j];
                        classId=(j-4)-i;
                    }
            }
            class_conf.push_back(max);
            clIds.push_back(classId);
            obj_conf.push_back(objConf);
            rect.emplace_back(x,y,width,height);
        }

        std::vector<int> idx;
        cv::dnn::NMSBoxes(rect, obj_conf, 0.3, 0.5, idx,1.0,5);
        boxes.reserve(idx.size());
        for (int i = 0; i < idx.size(); i++){
                BoundingBox box;
                box.x = rect[idx[i]].x;
                box.y = rect[idx[i]].y;
                box.width = rect[idx[i]].width;
                box.height = rect[idx[i]].height;
                box.obj_conf = obj_conf[idx[i]];
                box.class_conf = class_conf[idx[i]];
                box.classId = clIds[idx[i]];

                boxes.emplace_back(box);
        }
        //cv::Scalar color =    cv::Scalar(0, 255, 0);
        //cv::rectangle(frame, cv::Rect(b.x, b.y, b.width, b.height), color, 1);
        //std::string out_str = class_names[b.classId] + " "+std::to_string(b.conf);
        //cv::putText(frame, out_str,cv::Point(b.x, b.y+10),cv::FONT_HERSHEY_DUPLEX,.25,color, 1);
        return boxes;
    }

void ObjectDetection::setup_gpu_compute(){

    // Platform
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);
    if (all_platforms.size()==0) {
        throw std::runtime_error("Unable to find OpenCL platform");
        return;
    }
    cl::Platform default_platform=all_platforms[0];
    std::cerr << "Using platform: "<<default_platform.getInfo<CL_PLATFORM_NAME>()<<"\n";

    // GPU Device
    std::vector<cl::Device> gpu_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_GPU, &gpu_devices);
    if(gpu_devices.size()==0){
        throw std::runtime_error("No GPU Device found");
        return;
    }
    cl::Device default_device=gpu_devices[0];
    std::cerr<< "Using device: "<<default_device.getInfo<CL_DEVICE_NAME>()<<"\n";

    // Queue and context
    context = cl::Context(default_device);
    queue   = cl::CommandQueue(context, default_device);

    // Compile
    std::ifstream kernel_file("./image_functions.cl");
    std::string src( std::istreambuf_iterator<char>( kernel_file.rdbuf()), std::istreambuf_iterator<char>() );
       
    program = cl::Program(context, src);
    if (program.build(default_device)!=CL_SUCCESS) {
        throw std::runtime_error("\nOpenCL build failed:\n" + program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device));
        return;
    }
}

void ObjectDetection::gpu_resize_image(const cl::Image2D& img){
    cl::Image2D writeImage(context,CL_MEM_WRITE_ONLY,cl::ImageFormat(CL_RGB, CL_SNORM_INT8),model_image_width,model_image_height,0,nullptr);
    cl_int err;
    cl::Kernel resizeKernel(program, "nn_resize",&err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to make the kernel, error code: " << err << std::endl;
        exit(1);
    }
    resizeKernel.setArg(0, img);
    resizeKernel.setArg(1, writeImage);
    resizeKernel.setArg(2, 640);
    resizeKernel.setArg(3, 480);
    resizeKernel.setArg(4, model_image_width);
    resizeKernel.setArg(5, model_image_height);
    err = queue.enqueueNDRangeKernel(
        resizeKernel,
        cl::NullRange,
        cl::NDRange(model_image_width,model_image_height),
        cl::NullRange
    );
    if (err != CL_SUCCESS) {
        std::cerr << "enqueueNDRangeKernel Failed, error code: " << err << std::endl;
    }
    cl::detail::size_t_array origin;
    origin[0]=0;
    origin[1]=0;
    origin[2]=0;
    cl::detail::size_t_array region;
    region[0]=model_image_width;
    region[1]=model_image_height;
    region[2]=1;

    char* data = (char*)malloc(model_image_width*model_image_height*model_image_chnls);
    memset(data,20,model_image_width*model_image_height*model_image_chnls);
    queue.enqueueReadImage(writeImage, CL_TRUE, origin, region, 0,0,(void*) data);
    if (err != CL_SUCCESS) {
        std::cerr << "enqueueReadImage Failed, error code: " << err << std::endl;
    }
    queue.finish();
    cv::Mat rawData(model_image_width,model_image_height,CV_8SC3,(void*)data);
    cv::imwrite("test.jpg", rawData); // A JPG FILE IS BEING SAVED
    free(data);
}

}