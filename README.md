# Nocturne
Nocturne is a mobile night time object detection system built with the [Quartz64](https://wiki.pine64.org/wiki/Quartz64) SBC.


## Implementation Details

### Object Detection
Implemeted with [TensorFlow Lite](https://www.tensorflow.org/lite) library with quantized [Yolo V5](https://www.kaggle.com/models/kaggle/yolo-v5) model 

### Camera
Video feed fetched with the Linux userspace api, video for Linux

Hardware: [Arducam 1080P Day & Night Vision](https://www.amazon.com/Arducam-Computer-Automatic-Switching-All-Day/dp/B0829HZ3Q7/)

### Dipslay
Object detection info shown with SSD1306 display using I<sup>2</sup>C serial bus

Hardware: [SSD1306 128x64](https://www.amazon.com/dp/B072Q2X2LL?psc=1&ref=ppx_yo2ov_dt_b_product_details)

### Distance sensor
Simple usage of ultrasonic senor with GPIO to calculate distance

Hardware: [HC-SR04 Ultrasonic Sensor](https://www.sparkfun.com/products/15569) 
