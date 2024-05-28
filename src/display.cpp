#include "headers/display.h"
#include "headers/utils.h"
#include "headers/font.h"

#include <linux/i2c-dev.h>
#include <iostream>
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

        uint8_t cmd = SSD1306::CMD_STREAM;
        uint8_t result = 0;
        send(&cmd, 1);
        recieve(&result, 1);
        if (result == 0){
            throw std::runtime_error("I2C connection test failed");
        }
        if(configure()!=CONFIG_BUFFER_SIZE){
            throw std::runtime_error("SSD1306 configuratio failed");
        }        
    }

    SSD1306_Display::~SSD1306_Display(){
        turn_off();
        close(fd);
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
        uint8_t data[2]{SSD1306::CMD_STREAM,SSD1306::DISPLAY_ON};
        return send(data, 2);
    }

    int SSD1306_Display::turn_off(){
        uint8_t data[2]{SSD1306::CMD_STREAM,SSD1306::DISPLAY_OFF};
        return send(data, 2);
    }
    
    int SSD1306_Display::configure(){
        uint8_t data[CONFIG_BUFFER_SIZE]{
            SSD1306::CMD_STREAM,  
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
            SSD1306::HORIZ_FLIP,   
            SSD1306::SCAN_REVS,     
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

    void SSD1306_Display::clear_screen(){
        for(int i=0;i<SSD1306::COL_SIZE/8;i++){
            write_line("",i);
        }
    }

    int SSD1306_Display::write_line(const std::string&& str,const uint8_t&& line){
        uint8_t header_size{7};
        size_t str_size = str.size()*8;
        
        int data_size{SSD1306::LINE_SIZE+header_size};
        uint8_t data[data_size]={0};
        data[0]=SSD1306::CMD_SINGLE;
        data[1]= 0xB0 |(line&0x0F);
        data[2]=SSD1306::CMD_SINGLE;
        data[3]=0x00;
        data[4]=SSD1306::CMD_SINGLE;
        data[5]=0x10;
        data[6]=SSD1306::DATA_CONTROL;
        uint32_t offset=0;
        uint32_t asci_int;
        for(int i=0;i<str_size;i++){
            asci_int=static_cast<uint32_t>(str[i/8]);

            if(asci_int<0x20 || asci_int>0x7E){
                data[i+1]=font8x8[0x7F-32];
                continue;
            }
            asci_int=(asci_int-32)*8;
            offset =i%8;
            data[i+header_size]  =font8x8[asci_int+offset];
        }

        if(send(data,data_size)!=data_size){
            std::cerr<<"Failed to write line: "<<str<<std::endl;
            return -1;
        }else{
            return data_size;
        }

    }
}