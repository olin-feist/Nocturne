#include <cstdint>
#include <mutex>
namespace nocturne{
    const float MPuS{0.000001};
    const float CM_TO_IN{1/2.54};
    const uint32_t M_TO_CM{100};
    
    class UltraSonicSensor{
        float MAX_RANGE{157.48};
        float MIN_RANGE{1};
        float ERROR_THRESH{100};

        std::mutex sensor_lock;
        float SOUND_SPEED_uS{350*MPuS};
        int fd;
        int trig_fd;
        int echo_fd;
        uint32_t echo_pin;
        uint32_t trig_pin;
        int start();
    public:
        UltraSonicSensor(uint32_t,uint32_t);
        UltraSonicSensor() = delete;
        int getDistance();
    };
}
