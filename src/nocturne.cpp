#include "headers/nocturne.h"
#include "headers/utils.h"
#include "headers/sensor.h"
#include "headers/display.h"
#include "headers/detect.h"

#include <signal.h>
#include <stdio.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <fstream>

std::atomic<bool> stop;

nocturne::SSD1306_Display disp{"/dev/i2c-4"};

void sig_handle(int signum) {
        stop = 1;
}

void distance_thread(){
    nocturne::UltraSonicSensor dist_sensor(0,1);
    int dist{0};
    while(!stop){
        dist =dist_sensor.getDistance();
        if(dist==-1){
            utils::errno_exit("getDistance() failed");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{60});
        disp.write_line("Distance " + std::to_string(dist) + " (IN)",1);
    }
};

void detection_thread(){
    nocturne::ObjectDetection detector;
    
    try{
        detector=nocturne::ObjectDetection("/root/1.tflite");
    } catch (std::exception& e){
        std::cerr<<"Exception "<<typeid(e).name()<<" Thrown when construction ObjectDetection, what(): "<<e.what()<<std::endl;
        stop=1;
        return;
    }

    std::vector<std::string> class_names = utils::get_class_names("/root/data/coco_classes.txt");
    std::vector<nocturne::BoundingBox> objs;
    double elapsed_time_ms;
    int i;
    while(!stop){
        auto start = std::chrono::high_resolution_clock::now();

        objs=detector.detect();
        i=2;
        for(auto b : objs){
            disp.write_line("Obj:"+class_names[b.classId],i++);
        }
        disp.clear_after_line(i);
        auto end = std::chrono::high_resolution_clock::now();
        elapsed_time_ms = std::chrono::duration<double, std::milli>(end-start).count();
        disp.write_line("FPS: " + std::to_string(1000/elapsed_time_ms),0);
    }
}

int main(){
  
    signal(SIGINT,  sig_handle);
    signal(SIGKILL, sig_handle);
    signal(SIGTERM, sig_handle);

    std::thread detc_thread(detection_thread);
    std::thread dist_thread(distance_thread);
    dist_thread.join();
    detc_thread.join();
    return 0;
}