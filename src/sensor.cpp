#include "headers/sensor.h"
#include "headers/utils.h"
#include <linux/gpio.h>
#include <sys/ioctl.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <string.h>

namespace nocturne{
    UltraSonicSensor::UltraSonicSensor(uint32_t trig_pin_,uint32_t echo_pin_):
    trig_pin(trig_pin_),
    echo_pin(echo_pin_)
    {
        fd = utils::open_device("/dev/gpiochip1");

        gpio_v2_line_request out;
        memset(&out, 0, sizeof(out));
        out.offsets[0]=trig_pin;
        out.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
        out.num_lines=1;

        if (ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &out) < 0) {
             std::cout << "file open failed: " << errno << "\n";
            throw std::runtime_error("Ioctl output pin Error: GPIO_V2_GET_LINE_IOCTL");
        }
        trig_fd = out.fd;

        gpio_v2_line_request in;
        memset(&in, 0, sizeof(in));
        in.offsets[0]=echo_pin;
        in.config.flags = GPIO_V2_LINE_FLAG_INPUT;
        in.num_lines=1;

        if (ioctl(fd, GPIO_V2_GET_LINE_IOCTL, &in) < 0) {
            throw std::runtime_error("Ioctl input pin Error: GPIO_V2_GET_LINE_IOCTL");
        }
        echo_fd=in.fd;
    }

    int UltraSonicSensor::start(){
        struct gpio_v2_line_values high;
        high.bits=_BITULL(0);
        high.mask=_BITULL(0);
        if (ioctl(trig_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &high) < 0) {
           std::cerr<<"Ioctl error when setting high: GPIO_V2_LINE_SET_VALUES_IOCTL"<<std::endl;
           return 1;
        }

        std::this_thread::sleep_for(std::chrono::microseconds{10});

        struct gpio_v2_line_values low;
        low.bits|=0;
        low.mask|=_BITULL(0);
        if (ioctl(trig_fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &low) < 0) {
           std::cerr<<"Ioctl error when setting low: GPIO_V2_LINE_SET_VALUES_IOCTL"<<std::endl;
           return 1;
        }
        return 0;
    }

    int UltraSonicSensor::getDistance(){
        std::lock_guard<std::mutex> guard(sensor_lock);
        if(start()){return -1;}
        // Read the value of the GPIO line
        struct gpio_v2_line_values values;
        values.mask=_BITULL(0);
        bool val {0};
        
        while(val==0){
            values.bits=0;
            if (ioctl(echo_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values) < 0) {
                std::cerr << "Failed to read sensor GPIO value" << std::endl;
                return -1;
            }
            val=values.bits&_BITULL(0);
        }
        auto start = std::chrono::high_resolution_clock::now();
        while(val==1){
            values.bits=0;
            if (ioctl(echo_fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &values) < 0) {
                std::cerr << "Failed to read sensor GPIO value" << std::endl;
                return -1;
            }
            val=values.bits&_BITULL(0);
        }
        auto end = std::chrono::high_resolution_clock::now();
        double elapsed_time_uS = std::chrono::duration<double, std::micro>(end-start).count();
        elapsed_time_uS=(((elapsed_time_uS*SOUND_SPEED_uS)/2)*M_TO_CM)*CM_TO_IN;

        if(elapsed_time_uS>MAX_RANGE+ERROR_THRESH){
            return 0;
        } else if(elapsed_time_uS>MAX_RANGE){
            return MAX_RANGE;
        }else if(elapsed_time_uS<MIN_RANGE){
            return MIN_RANGE;
        }

        return elapsed_time_uS;
    }
}