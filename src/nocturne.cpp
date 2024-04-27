/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"

/*-------------------------------------------
                  Functions
-------------------------------------------*/
int open_device(const std::string& device){
  int fd =  open(device.c_str(), O_RDONLY);
  if (fd == -1){
    std::cerr<<"failed to open /dev/video1"<<std::endl;
  }
  return fd;
}

void configure_camera(const int& fd){
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = 640;
  fmt.fmt.pix.height = 480;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt)) {
    std::cerr<<"VIDIOC_S_FMT"<<std::endl;
    exit(EXIT_FAILURE);
  }
}

int get_frame(const int& fd){
  while(true){
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    
    if (-1 ==  ioctl(fd, VIDIOC_DQBUF, &buf)) {
      std::cerr<<"VIDIOC_DQBUF"<<std::endl;
      exit(EXIT_FAILURE);
    }
    uint8_t* buffer;
    buffer = (uint8_t*) mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    std::cout<<buffer[0]<<std::endl;

    munmap(buffer, buf.length);
    ioctl(fd, VIDIOC_QBUF, &buf);
  }
}

int main(){
  // Init Model
  tfModelPtr model = tflite::FlatBufferModel::BuildFromFile("1.tflite");
  if(!model){
      printf("Failed to Load model\n");
      return 1;
  }

  struct v4l2_input video_input;
  int index;

  int fd = open_device("/dev/video1");
  if (fd == -1){exit(EXIT_FAILURE);}

  configure_camera(fd);
  get_frame(fd);
  
  /*memset(&video_input, 0, sizeof(video_input));
  video_input.index = index;

  if (-1 == ioctl(fd, VIDIOC_ENUMINPUT, &video_input)) {
    std::cerr<<"VIDIOC_ENUMINPUT"<<std::endl;
    exit(EXIT_FAILURE);
  }
  
  std::cout<<"Current input: "<<video_input.name <<std::endl;*/

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