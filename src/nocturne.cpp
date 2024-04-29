/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"
#include "headers/capture.h"
#include "headers/utils.h"
/*-------------------------------------------
                  Functions
-------------------------------------------*/

void print_camera_info(const v4l2_capability& capability){
  std::cout<<"Driver: "<<capability.driver<<std::endl;
  std::cout<<"Card: "<<capability.card<<std::endl;
  std::cout<<"Bus info: "<<capability.bus_info<<std::endl;
  std::cout<<"Version: "<<capability.version<<std::endl;
  std::cout<<"Capabilities: "<<capability.capabilities<<std::endl;
  std::cout<<"Device Caps: "<<capability.device_caps<<std::endl;
}

int main(){
  // Init Model
  tfModelPtr model = tflite::FlatBufferModel::BuildFromFile("1.tflite");
  if(!model){
      printf("Failed to Load model\n");
      return 1;
  }

  nocturne::Capture cam1("/dev/video1");
  char* buf=0;
  u_int32_t size;
  cam1.get_frame(&buf,size);
  std::ofstream outFile;
  outFile.open("output.jpeg", std::ios::binary | std::ios::trunc);
  outFile.write(buf, size);
  outFile.close();
  
  //free(buf);


  // Interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);

  
  interpreter->AllocateTensors();

  float* input = interpreter->typed_input_tensor<float>(0);
  
  interpreter->Invoke();

  float* output = interpreter->typed_output_tensor<float>(0);

  return 0;
}