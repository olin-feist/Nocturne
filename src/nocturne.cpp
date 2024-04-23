/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"

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

  struct v4l2_input video_input;
  int index;
  int fd = open("/dev/video0", O_RDONLY);
  if (-1 == ioctl(fd, VIDIOC_G_INPUT, &index)) {
    std::cerr<<"VIDIOC_G_INPUT"<<std::endl;
    exit(EXIT_FAILURE);
  }

  memset(&video_input, 0, sizeof(video_input));
  video_input.index = index;

  if (-1 == ioctl(fd, VIDIOC_ENUMINPUT, &video_input)) {
    std::cerr<<"VIDIOC_ENUMINPUT"<<std::endl;
    exit(EXIT_FAILURE);
  }
  
  std::cout<<"Current input: "<<video_input.name <<std::endl;

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