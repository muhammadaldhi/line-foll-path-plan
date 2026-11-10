// ESP32-C3 foundation configuration
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

// Capacity constants
constexpr uint8_t SENSOR_COUNT = 14;
constexpr uint8_t MEMORY_COUNT = 16;
constexpr uint8_t CHECKPOINT_COUNT = 30;
constexpr uint16_t STEP_COUNT = 150;
constexpr uint8_t CALIBRATION_PROFILE_COUNT = 5;

// Motor pins
constexpr uint8_t PWMR2 = 0;
constexpr uint8_t PWMR1 = 1;

// Selector pins
constexpr uint8_t SEL1 = 2;
constexpr uint8_t SEL0 = 3;
constexpr uint8_t OUT_MUX = 4;
constexpr uint8_t PWML1 = 5;
constexpr uint8_t SEL3 = 6;
constexpr uint8_t PWML2 = 7;
constexpr uint8_t SEL2 = 8;

// Optional hardware pins (-1 means unassigned)
constexpr int8_t SERVO_L_PIN = -1;
constexpr int8_t SERVO_R_PIN = -1;
constexpr int8_t ENC_L_A_PIN = -1;
constexpr int8_t ENC_L_B_PIN = -1;
constexpr int8_t ENC_R_A_PIN = -1;
constexpr int8_t ENC_R_B_PIN = -1;
constexpr int8_t BUZZER_PIN = -1;

// ST7789 display pins. CS is tied low; TFT_eSPI setup must use TFT_CS -1.
constexpr uint8_t TFT_SCK = 9;
constexpr uint8_t TFT_MOSI = 10;
constexpr uint8_t TFT_RES = 20;
constexpr uint8_t TFT_DC = 21;
constexpr int8_t TFT_CS = -1;

#endif // CONFIG_H
