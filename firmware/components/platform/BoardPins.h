#pragma once

// =====================================================
// WAVESHARE ESP32-S3-TOUCH-LCD-4.3
// Values taken from official ESP-IDF 03_SD_Test demo.
// =====================================================

// I2C
constexpr int BOARD_I2C_SCL = 19;
constexpr int BOARD_I2C_SDA = 18;

constexpr int BOARD_I2C_PORT = 0;
constexpr int BOARD_I2C_FREQ_HZ = 400000;

// SD card - SPI
constexpr int BOARD_SD_MOSI = 35;
constexpr int BOARD_SD_MISO = 37;
constexpr int BOARD_SD_CLK  = 36;
constexpr int BOARD_SD_CS   = 34;

// Filesystem mount point
constexpr const char* BOARD_SD_MOUNT_POINT =
    "/sdcard";