/*-------------------------------------------
                Includes
-------------------------------------------*/
#include "headers/nocturne.h"

/*-------------------------------------------
                  Functions
-------------------------------------------*/
int open_device(const std::string& device){
  int fd =  open(device.c_str(), O_RDWR);
  if (fd == -1){
    std::cerr<<"failed to open "<<device<<std::endl;
  }
  return fd;
}

void print_camera_info(const v4l2_capability& capability){
  std::cout<<"Driver: "<<capability.driver<<std::endl;
  std::cout<<"Card: "<<capability.card<<std::endl;
  std::cout<<"Bus info: "<<capability.bus_info<<std::endl;
  std::cout<<"Version: "<<capability.version<<std::endl;
  std::cout<<"Capabilities: "<<capability.capabilities<<std::endl;
  std::cout<<"Device Caps: "<<capability.device_caps<<std::endl;
}

void configure_camera(const int& fd){

  v4l2_capability capability;
  if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
      std::cerr<<"Ioctl Error: VIDIOC_QUERYCAP"<<std::endl;
      exit(EXIT_FAILURE);
  }

  print_camera_info(capability);

  if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    std::cerr<<"No video capture device"<<std::endl;
    exit(EXIT_FAILURE);
  }

  if (!(capability.capabilities & V4L2_CAP_STREAMING)) {
    std::cerr<<"Does not support streaming i/o"<<std::endl;
    exit(EXIT_FAILURE);
  }

  v4l2_format image_fmt;
  image_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  image_fmt.fmt.pix.width = WIDTH;
  image_fmt.fmt.pix.height = HEIGHT;
  image_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  image_fmt.fmt.pix.field = V4L2_FIELD_ANY;
  if (-1 == ioctl(fd, VIDIOC_S_FMT, &image_fmt)) {
    std::cerr<<"Ioctl Error: VIDIOC_S_FMT"<<std::endl;
    exit(EXIT_FAILURE);
  }

  v4l2_requestbuffers requestBuffer = {0};
  requestBuffer.count = 1;
  requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  requestBuffer.memory = V4L2_MEMORY_MMAP;

  if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) == -1){
      std::cerr<<"Ioctl Error: VIDIOC_REQBUFS"<<std::endl;
      exit(EXIT_FAILURE);
  }
}

int get_frame(const int& fd){
  // Query
  v4l2_buffer query_buf {0} ;
  query_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  query_buf.memory = V4L2_MEMORY_MMAP;
  query_buf.index = 0;
  
  if (ioctl(fd, VIDIOC_QUERYBUF, &query_buf)== -1) {
    std::cerr<<"VIDIOC_QUERYBUF"<<std::endl;
    printf("Error %s", strerror(errno));
    return 1;
  }

  char* buffer = (char*) mmap(NULL, query_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, query_buf.m.offset);
  if(buffer == MAP_FAILED){
      std::cerr<<"Failed to map buffer"<<std::endl;
      return 1;
  }




  v4l2_buffer bufferinfo;
  memset(&bufferinfo, 0, sizeof(bufferinfo));
  bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  bufferinfo.memory = V4L2_MEMORY_MMAP;
  bufferinfo.index = 0;

  // Que
  if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) == -1){
      std::cerr<<"Failed to Que Buffer, ioctl: VIDIOC_QBUF"<<std::endl;
      return 1;
  }

  // Stream
  if(ioctl(fd, VIDIOC_STREAMON, &bufferinfo.type) == -1){
    std::cerr<<"Failed to to turn on stream, icotl: VIDIOC_STREAMON"<<std::endl;
    return 1;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  // Dequeue
  if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) == -1){
      std::cerr<<"VIDIOC_DQBUF"<<std::endl;
      return 1;
  }

  std::cout << "Buffer read: " << (double)bufferinfo.bytesused / 1024 << " KBytes of data" << std::endl;

  std::ofstream outFile;
  outFile.open("output.jpeg", std::ios::binary | std::ios::trunc);
  outFile.write(buffer, (double)bufferinfo.bytesused);

  outFile.close();

  // end streaming
  if(ioctl(fd, VIDIOC_STREAMOFF, &bufferinfo.type) < 0){
    std::cerr<<"Failed to end stream, icoctl: VIDIOC_STREAMOFF"<<std::endl;
    return 1;
  }

  return 0;
}

int main(){
  // Init Model
  tfModelPtr model = tflite::FlatBufferModel::BuildFromFile("1.tflite");
  if(!model){
      printf("Failed to Load model\n");
      return 1;
  }

  v4l2_input video_input;
  int index;

  int fd = open_device("/dev/video0");
  if (fd == -1){exit(EXIT_FAILURE);}

  configure_camera(fd);
  if(get_frame(fd)){
      std::cerr<<"Getting frame failed"<<std::endl;
      exit(EXIT_FAILURE);
  }
  
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