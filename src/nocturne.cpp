/*-------------------------------------------
                                Includes
-------------------------------------------*/
#include "headers/nocturne.h"
#include "headers/utils.h"
#include <signal.h>
/*-------------------------------------------
                                    Functions
-------------------------------------------*/

volatile sig_atomic_t stop;

void sigint_handle(int signum) {
        stop = 1;
}

BoundingBox::BoundingBox(u_int32_t x_,u_int32_t y_,u_int32_t width_,u_int32_t height_,u_int32_t id_,float conf_){
    x=x_;
    y=y_;
    width=width_;
    height=height_;
    classId=id_;
    conf=conf_;
}

void BoundingBox::print(){
    std::cout<<"X:\t "<<x<<std::endl;
    std::cout<<"Y:\t "<<y<<std::endl;
    std::cout<<"Width:\t "<<width<<std::endl;
    std::cout<<"Height:\t "<<height<<std::endl;
    std::cout<<"Conf:\t "<<conf<<std::endl;
    std::cout<<"Class ID:\t "<<classId<<std::endl;
}

ObjectDetection::ObjectDetection(const std::string& path):
cam1("/dev/video0")
{
    // Init Model
    model = tflite::FlatBufferModel::BuildFromFile(path.c_str());
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
}

ObjectDetection::~ObjectDetection(){
    if(buf!=NULL)
        free(buf);
}

std::vector<BoundingBox> ObjectDetection::detect(){
    cam1.get_frame(&buf,size);
    cv::Mat rawData(1,size,CV_8SC1,(void*)buf);
    cv::Mat frame= cv::imdecode(rawData,cv::IMREAD_UNCHANGED);
    cv::resize(frame, frame, cv::Size(model_image_width, model_image_height), 0, 0, cv::INTER_AREA);
    
    input_tensor = interpreter->typed_input_tensor<float>(0);

    // Copy float image into input tensor
    cv::Mat fimage;
    frame.convertTo(fimage, CV_32FC3,1.0/128.0,-1);
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
        std::vector<float> conf;
        std::vector<cv::Rect> rect;

        float objConf;
        u_int32_t x,y,width,height,classId;
        for(int i=0;i<model_num_canidates*model_num_classes;i+=model_num_classes){
            // Box
            objConf = output_tensor[i+4];
            if(objConf<0.3)
                continue;
            
            width     = output_tensor[i + 2]*320;
            height    = output_tensor[i + 3]*320;
            x = (output_tensor[i] * 320 )-width /2;
            y = (output_tensor[i + 1] * 320)-height /2;

            // Class
            float max=-1.0;
            for(int j=i+5;j<i+80;j++){
                    if(output_tensor[j]>max){
                        max=output_tensor[j];
                        classId=(j-4)-i;
                    }
            }
            max*=objConf;
            clIds.push_back(classId);
            conf.push_back(objConf);
            rect.emplace_back(x,y,width,height);
        }

        std::vector<int> idx;
        cv::dnn::NMSBoxes(rect, conf, 0.3, 0.5, idx);
        boxes.reserve(idx.size());
        for (int i = 0; i < idx.size(); i++){
                BoundingBox box;
                box.x = rect[idx[i]].x;
                box.y = rect[idx[i]].y;
                box.width = rect[idx[i]].width;
                box.height = rect[idx[i]].height;
                box.conf = conf[idx[i]];
                box.classId = clIds[idx[i]];

                boxes.emplace_back(box);
        }
        return boxes;
}

int main(){
    signal(SIGINT, sigint_handle);

    ObjectDetection detector("1.tflite");
    /*try{
        detector=ObjectDetection("1.tflite");
    } catch (std::exception& e){
        std::cerr<<"Exception "<<typeid(e).name()<<" Thrown when construction ObjectDetection, what(): "<<e.what()<<std::endl;
        return 1;
    }*/
    std::vector<std::string> class_names = utils::get_class_names("data/coco_classes.txt");

    while(!stop){
        auto objs = detector.detect();
		std::cout<<"\033[2J\033[1;2H";
        for(auto b : objs){
            std::cout<<"Object: "<<class_names[b.classId]<<" X: "<<b.x<<" Y: "<<b.y<<" Conf: "<<b.conf<<std::endl;
            //b.print();
            //cv::Scalar color =    cv::Scalar(0, 255, 0);
            //cv::rectangle(frame, cv::Rect(b.x, b.y, b.width, b.height), color, 1);
            //std::string out_str = class_names[b.classId] + " "+std::to_string(b.conf);
            //cv::putText(frame, out_str,cv::Point(b.x, b.y+10),cv::FONT_HERSHEY_DUPLEX,.25,color, 1);
        }
    }

    return 0;
}