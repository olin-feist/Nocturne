# Nocturne
Nocturne is a mobile night time object detection and distance system built with the [Quartz64](https://wiki.pine64.org/wiki/Quartz64) SBC and written entirely in C++.

## Overview
This project involves the development of a compact, integrated system that combines a single-board computer with a camera, display, and ultrasonic sensor. The system leverages TensorFlow Lite to perform real-time object detection and displays both the detected objects and a single point distance on a screen.

## Implementation Details

### Object Detection
Implemeted with [TensorFlow Lite](https://www.tensorflow.org/lite) library and a quantized [Yolo V5](https://www.kaggle.com/models/kaggle/yolo-v5) model 

### Camera
Video feed fetched with the Linux userspace api, video for Linux. Image decoding and croping done with [OpenCV](https://opencv.org/)

### Dipslay
Object detection info shown with SSD1306 display using I<sup>2</sup>C serial bus

### Distance sensor
Simple usage of ultrasonic senor with GPIO for communication

### Software

| | |
| -------- | ------- |
| **Architecture**  | Arm64    |
| **OS** | Manjaro Linux     |
| **Dependencies**    |  Linux userspace API, TensorFlow Lite, OpenCV    |

### Hardware

[Quartz64 Model B](https://wiki.pine64.org/wiki/Quartz64)

[SSD1306 128x64](https://www.amazon.com/dp/B072Q2X2LL?psc=1&ref=ppx_yo2ov_dt_b_product_details)

[HC-SR04 Ultrasonic Sensor](https://www.sparkfun.com/products/15569) 

[Arducam 1080P Day & Night Vision](https://www.amazon.com/Arducam-Computer-Automatic-Switching-All-Day/dp/B0829HZ3Q7/)

### Architecure Diagram

![Image](https://i.imgur.com/Le4mXf0.png)
