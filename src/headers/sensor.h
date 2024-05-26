#include <cstdint>

namespace nocturne{
    class UltraSonicSensor{
        int fd;
        int trig_fd;
        uint32_t echo_pin;
        uint32_t trig_pin;
    public:
        UltraSonicSensor(uint32_t,uint32_t);
        UltraSonicSensor() = delete;
        int start();
        int stop();
        uint32_t getDistance();
    };
}
