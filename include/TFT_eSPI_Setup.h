// Copy these definitions into a project-local TFT_eSPI User_Setup.h selected by
// User_Setup_Select.h, or configure equivalent build flags. Do not edit library files.
// ESP32-C3 + 240x240 ST7789, CS tied permanently to GND.
#define ST7789_DRIVER
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define TFT_SCLK 9
#define TFT_MOSI 10
#define TFT_MISO -1
#define TFT_CS -1
#define TFT_DC 21
#define TFT_RST 20
#define SPI_FREQUENCY 40000000
