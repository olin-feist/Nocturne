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
  cv::Mat frame_1= cv::imdecode(rawData,cv::IMREAD_UNCHANGED);
  cv::Mat frame;
  cv::resize(frame_1, frame, cv::Size(416, 416), 0, 0, cv::INTER_AREA);


  // Interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::InterpreterBuilder(*model, resolver)(&interpreter);
  
  interpreter->AllocateTensors();

  // Get the number of output tensors
  int outputTensorCount = interpreter->outputs().size();
  std::cout << "Number of output tensors: " << outputTensorCount << std::endl;

  for (int i = 0; i < outputTensorCount; ++i) {
      TfLiteTensor* outputTensor = interpreter->output_tensor(i);
      std::cout << "Output tensor #" << i << ":" << std::endl;
      std::cout << "  Name: " << outputTensor->name << std::endl;
      std::cout << "  Type: " << outputTensor->type << std::endl;
      std::cout << "  Dimensions: ";
      for (int d = 0; d < outputTensor->dims->size; ++d) {
          std::cout << outputTensor->dims->data[d] << " ";
      }
      std::cout << std::endl;
  }

  float* input = interpreter->typed_input_tensor<float>(0);
  // Copy float image into input tensor
  cv::Mat fimage;
  frame.convertTo(fimage, CV_32FC3);
  cv::cvtColor(fimage, fimage, cv::COLOR_BGR2RGB);
  cv::imwrite("some.jpg", fimage);
  memcpy(input, fimage.data,sizeof(float) * 416 * 416 * 3);

  if (interpreter->Invoke() != kTfLiteOk) {
    std::cerr<<"Error invoking"<<std::endl;
    return 1;
  }
  float* output = interpreter->typed_output_tensor<float>(0);
  // Create a CV_32FC3 OpenCV Mat from the output tensor data
  cv::Mat output_image(416, 416, CV_32FC1, output);

  // Convert the output image to the range [0, 255]
  output_image *= 255.0;

  // Convert the CV_32FC3 image to CV_8UC3 (unsigned char)
  output_image.convertTo(output_image, CV_8UC1);

  // Save the image to JPEG file
  cv::imwrite("imgOut.jpg", output_image);
  //cv::imwrite("imgOut.jpeg",  cv::Mat(416, 416, CV_32FC3, (void*)output));

  free(buf);
  return 0;
}