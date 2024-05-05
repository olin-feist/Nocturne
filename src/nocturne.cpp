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
  std::cout<<std::hex<<(int)buf[0]<<std::hex<<(int)buf[1]<<std::hex<<(int)buf[2]<<std::hex<<(int)buf[3]<<std::endl;

  cv::Mat rawData(1,sizeof(buf),CV_8SC1,(void*)buf);
  cv::Mat frame= cv::imdecode(rawData,cv::IMREAD_COLOR);


  // Interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);

  // Get input tensor details
  interpreter->AllocateTensors();
  char* input = interpreter->typed_input_tensor<char>(0);
  //memcpy(input, buf, size);
  interpreter->Invoke();

  float* output = interpreter->typed_output_tensor<float>(0);

  utils::save_jpeg("output.jpeg",buf,size);
  free(buf);
  return 0;
}