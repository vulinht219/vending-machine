#pragma once

// =====================================================
// WAVESHARE ESP32-S3-TOUCH-LCD-4.3
// =====================================================

// -----------------------------------------------------
// Shared peripheral I2C bus
//
// Used by:
// - CH422G
// - GT911 touch
// - DS3231 RTC
// - PCA9685 servo controller
// -----------------------------------------------------

constexpr int BOARD_I2C_SCL = 9;
constexpr int BOARD_I2C_SDA = 8;

constexpr int BOARD_I2C_PORT = 0;
constexpr int BOARD_I2C_FREQ_HZ = 400000;


// -----------------------------------------------------
// SD card - SPI
// -----------------------------------------------------

constexpr int BOARD_SD_MOSI = 35;
constexpr int BOARD_SD_MISO = 37;
constexpr int BOARD_SD_CLK  = 36;
constexpr int BOARD_SD_CS   = 34;


// -----------------------------------------------------
// Filesystem
// -----------------------------------------------------

constexpr const char* BOARD_SD_MOUNT_POINT =
    "/sdcard";