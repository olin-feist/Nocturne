#include <cstdint>
#include <string>

namespace nocturne{
    namespace SSD1306{
        //I2C ADDRESS
        constexpr uint8_t I2C_ADDR       {0x3C};

        //CONTROL CODES
        constexpr uint8_t CMD_STREAM     {0x00};
        constexpr uint8_t CMD_SINGLE     {0x80};
        constexpr uint8_t DATA_CONTROL   {0x40};
        constexpr uint8_t DISPLAY_OFF    {0xae};
        constexpr uint8_t DISPLAY_ON     {0xaf};
        constexpr uint8_t HORIZ_NORM     {0xa0};
        constexpr uint8_t HORIZ_FLIP     {0xa1};
        constexpr uint8_t RESUME_RAM     {0xa4};
        constexpr uint8_t IGNORE_RAM     {0xa5};
        constexpr uint8_t DISP_NORM      {0xa6};
        constexpr uint8_t DISP_INVERSE   {0xa7};
        constexpr uint8_t MULTIPLEX      {0xa8};
        constexpr uint8_t VERT_OFFSET    {0xd3};
        constexpr uint8_t CLK_SET        {0xd5};
        constexpr uint8_t PRECHARGE      {0xd9};
        constexpr uint8_t COM_PIN        {0xda};
        constexpr uint8_t DESELECT_LV    {0xdb};
        constexpr uint8_t CONTRAST       {0x81};
        constexpr uint8_t DISABLE_SCROLL {0x2e};
        constexpr uint8_t ENABLE_SCROLL  {0x2f};
        constexpr uint8_t PAGE_NUMBER    {0xb0};
        constexpr uint8_t LOW_COLUMN     {0x00};
        constexpr uint8_t HIGH_COLUMN    {0x10};
        constexpr uint8_t START_LINE     {0x40};
        constexpr uint8_t CHARGE_PUMP    {0x8d};
        constexpr uint8_t SCAN_NORM      {0xc0};
        constexpr uint8_t SCAN_REVS      {0xc8};
        constexpr uint8_t MEMORY_MODE    {0x20};
        constexpr uint8_t SET_COL_ADDR   {0x21};
        constexpr uint8_t SET_PAGE_ADDR  {0x22};
        constexpr uint8_t HORI_MODE      {0x00};
        constexpr uint8_t VERT_MODE      {0x01};
        constexpr uint8_t PAGE_MODE      {0x02};

        //BYTE VALUES
        constexpr uint8_t  LINE_SIZE     {128};
        constexpr uint8_t  COL_SIZE      {64};
        constexpr uint16_t TOTAL_SIZE    {LINE_SIZE*(COL_SIZE/8)};
    }

    class  SSD1306_Display{
        int fd;
        int addr{SSD1306::I2C_ADDR};
        uint32_t CONFIG_BUFFER_SIZE{28};
        int send(uint8_t*, uint16_t);
        int recieve(uint8_t* data, uint16_t len);

        int configure();
    public:
        SSD1306_Display(const std::string&);
        ~SSD1306_Display();
        int turn_on();
        int turn_off();
        void clear_screen();
        void clear_after_line(const int&);
        
        int write_line(const std::string&&,const uint8_t&&);
    };

}