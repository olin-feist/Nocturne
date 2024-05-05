/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"
#include "headers/capture.h"
#include "headers/utils.h"
/*-------------------------------------------
                  Functions
-------------------------------------------*/


int main(){
  // Init Model
  tfModelPtr model = tflite::FlatBufferModel::BuildFromFile("1.tflite");
  if(!model){
      printf("Failed to Load model\n");
      return 1;
  }

  nocturne::Capture cam1("/dev/video0");
  char* buf=0;
  u_int32_t size;
  cam1.get_frame(&buf,size);
  utils::save_jpeg("output.jpeg",buf,size);

  cv::Mat rawData(1,size,CV_8SC1,(void*)buf);
  cv::Mat frame= cv::imdecode(rawData,cv::IMREAD_UNCHANGED);
  cv::resize(frame, frame, cv::Size(320, 320), 0, 0, cv::INTER_AREA);
  cv::imwrite("some.jpg", frame);

  // Interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);
  
  interpreter->AllocateTensors();

  TfLiteTensor* intputtensor = interpreter->input_tensor(0);
  std::cout << "  Name: " << intputtensor->name << std::endl;
  std::cout << "  Type: " << intputtensor->type << std::endl;
  std::cout << "  Dimensions: ";
  for (int d = 0; d < intputtensor->dims->size; ++d) {
      std::cout << intputtensor->dims->data[d] << " ";
  }
  std::cout<<std::endl;

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
  float conf=-1.0;
  float x=0;
  float y=0;
  for(int i =0;i<6300*85;i+=85){
    if(conf<output[i+4]){
      conf=output[i+4];
      x=output[i]-output[i+2]/2;
      y=output[i+1]-output[i+3]/2;
    }
  }
  std::cout<<"Conf: "<<conf<<std::endl;
  std::cout<<"X: "<<x<<std::endl;
  std::cout<<"Y: "<<y<<std::endl;

  free(buf);
  return 0;
}