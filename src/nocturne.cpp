/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"
#include "headers/capture.h"
#include "headers/utils.h"
/*-------------------------------------------
                  Functions
-------------------------------------------*/

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


std::vector<BoundingBox> get_boxes(u_int32_t k_,float* out,int canidate_boxes){
    std::vector<BoundingBox> boxes;

    std::vector<int> clIds;
    std::vector<float> conf;
    std::vector<cv::Rect> rect;

    float objConf;
    u_int32_t x,y,width,height,classId;
    for(int i=0;i<canidate_boxes*85;i+=85){
      // Box
      objConf = out[i+4];
      if(objConf<0.3)
        continue;
      
      width   = out[i + 2]*320;
      height  = out[i + 3]*320;
      x = (out[i] * 320 )-width /2;
      y = (out[i + 1] * 320)-height /2;

      // Class
      float max=-1.0;
      for(int j=i+5;j<i+80;j++){
          if(out[j]>max){
            max=out[j];
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
  // Init Model
  tfModelPtr model = tflite::FlatBufferModel::BuildFromFile("1.tflite");
  if(!model){
      printf("Failed to Load model\n");
      return 1;
  }
  std::vector<std::string> class_names = utils::get_class_names("data/coco_classes.txt");

  nocturne::Capture cam1("/dev/video2");
  char* buf=0;
  u_int32_t size;

  // Interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);
  
  interpreter->AllocateTensors();

  while(true){
  cam1.get_frame(&buf,size);
  utils::save_jpeg("output.jpeg",buf,size);

  cv::Mat rawData(1,size,CV_8SC1,(void*)buf);
  cv::Mat frame= cv::imdecode(rawData,cv::IMREAD_UNCHANGED);
  cv::resize(frame, frame, cv::Size(320, 320), 0, 0, cv::INTER_AREA);




  /*TfLiteTensor* intputtensor = interpreter->input_tensor(0);
  std::cout << "  Name: " << intputtensor->name << std::endl;
  std::cout << "  Type: " << intputtensor->type << std::endl;
  std::cout << "  Dimensions: ";
  for (int d = 0; d < intputtensor->dims->size; ++d) {
      std::cout << intputtensor->dims->data[d] << " ";
  }
  std::cout<<std::endl;
  */
  float* input = interpreter->typed_input_tensor<float>(0);

  // Copy float image into input tensor
  cv::Mat fimage;
  frame.convertTo(fimage, CV_32FC3,1.0/128.0,-1);
  cv::cvtColor(fimage, fimage, cv::COLOR_BGR2RGB);

  memcpy(input, fimage.data,sizeof(float) * 320 * 320 * 3);

  if (interpreter->Invoke() != kTfLiteOk) {
    std::cerr<<"Error invoking"<<std::endl;
    return 1;
  }

  float* output = interpreter->typed_output_tensor<float>(0);
  auto bb = get_boxes(5,output,6300);
  for(auto b : bb){
    //b.print();
    cv::Scalar color =  cv::Scalar(0, 255, 0);
    cv::rectangle(frame, cv::Rect(b.x, b.y, b.width, b.height), color, 1);
    std::string out_str = class_names[b.classId] + " "+std::to_string(b.conf);
    cv::putText(frame, out_str,cv::Point(b.x, b.y+10),cv::FONT_HERSHEY_DUPLEX,.25,color, 1);
  }
  //cv::resize(frame, frame, cv::Size(1920, 1080), 0, 0, cv::INTER_AREA);
  
  cv::imwrite("some.jpg", frame);
  }
  free(buf);
  return 0;
}