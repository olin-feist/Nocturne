#include "headers/display.h"
#include "headers/utils.h"
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace nocturne{
    SSD1306_Display::SSD1306_Display(){
        fd = utils::open_device("/dev/i2c-4");
        if(fd ==-1){
            throw std::runtime_error("Failed to open I2C driver");
        }

        if (ioctl(fd, I2C_SLAVE, addr) < 0) {
           throw std::runtime_error("Failed to open set adress of I2C device");
        }

        uint8_t cmd = SSD1306::CONTROL_BYTE;
        uint8_t result = 0;
        send(&cmd, 1);
        recieve(&result, 1);
        if (result == 0){
            throw std::runtime_error("I2C connection test failed");
        }
        configure();
    }

    int SSD1306_Display::send(uint8_t* data, uint16_t len){
        if(data==NULL || len ==0){return 0;}
        return write(fd,data,len);
    }

    int SSD1306_Display::recieve(uint8_t* data, uint16_t len){
        if(data==NULL || len ==0){return 0;}
        return read(fd,data,len);
    }

    int SSD1306_Display::turn_on(){
        uint8_t data[2]{SSD1306::CONTROL_BYTE,SSD1306::DISPLAY_ON};
        return send(data, 2);
    }

    int SSD1306_Display::turn_off(){
            uint8_t data[2]{SSD1306::CONTROL_BYTE,SSD1306::DISPLAY_OFF};
        return send(data, 2);
    }
    
    int SSD1306_Display::configure(){
        uint8_t data[CONFIG_BUFFER_SIZE]{
            SSD1306::CONTROL_BYTE,  
            SSD1306::DISPLAY_OFF,   
            SSD1306::DISP_NORM,     
            SSD1306::CLK_SET,       
            0x80,                   
            SSD1306::MULTIPLEX,     
            64 - 1,                 
            SSD1306::VERT_OFFSET,   
            0,                      
            SSD1306::START_LINE,    
            SSD1306::CHARGE_PUMP,   
            0x14,                   
            SSD1306::MEMORY_MODE,   
            SSD1306::PAGE_MODE,     
            SSD1306::HORIZ_NORM,   
            SSD1306::SCAN_NORM,     
            SSD1306::COM_PIN,        
            0x12,                  
            SSD1306::CONTRAST,      
            0x7f,                   
            SSD1306::PRECHARGE,     
            0xf1,                   
            SSD1306::DESELECT_LV,                   
            0x40,                   
            SSD1306::RESUME_RAM,    
            SSD1306::DISP_NORM,     
            SSD1306::DISPLAY_ON,                 
            SSD1306::DISABLE_SCROLL 
        };
       
        
        return send(data, CONFIG_BUFFER_SIZE);
    }
}