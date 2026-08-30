#pragma once


// =====================================================
// SHARED I2C BUS
// =====================================================

constexpr int BOARD_I2C_SCL =
    9;

constexpr int BOARD_I2C_SDA =
    8;

constexpr int BOARD_I2C_PORT =
    0;

constexpr int BOARD_I2C_FREQ_HZ =
    400000;


// =====================================================
// SD CARD
// =====================================================

constexpr int BOARD_SD_MOSI =
    35;

constexpr int BOARD_SD_MISO =
    37;

constexpr int BOARD_SD_CLK =
    36;

constexpr int BOARD_SD_CS =
    34;

constexpr const char* BOARD_SD_MOUNT_POINT =
    "/sdcard";


// =====================================================
// LCD
// =====================================================

constexpr int BOARD_LCD_WIDTH =
    800;

constexpr int BOARD_LCD_HEIGHT =
    480;

constexpr int BOARD_LCD_PIXEL_CLOCK_HZ =
    16 * 1000 * 1000;


// =====================================================
// LCD RGB CONTROL
// =====================================================

constexpr int BOARD_LCD_HSYNC =
    46;

constexpr int BOARD_LCD_VSYNC =
    3;

constexpr int BOARD_LCD_DE =
    5;

constexpr int BOARD_LCD_PCLK =
    7;

constexpr int BOARD_LCD_DISP =
    -1;


// =====================================================
// LCD RGB DATA
// =====================================================

constexpr int BOARD_LCD_DATA0 =
    14;

constexpr int BOARD_LCD_DATA1 =
    38;

constexpr int BOARD_LCD_DATA2 =
    18;

constexpr int BOARD_LCD_DATA3 =
    17;

constexpr int BOARD_LCD_DATA4 =
    10;

constexpr int BOARD_LCD_DATA5 =
    39;

constexpr int BOARD_LCD_DATA6 =
    0;

constexpr int BOARD_LCD_DATA7 =
    45;

constexpr int BOARD_LCD_DATA8 =
    48;

constexpr int BOARD_LCD_DATA9 =
    47;

constexpr int BOARD_LCD_DATA10 =
    21;

constexpr int BOARD_LCD_DATA11 =
    1;

constexpr int BOARD_LCD_DATA12 =
    2;

constexpr int BOARD_LCD_DATA13 =
    42;

constexpr int BOARD_LCD_DATA14 =
    41;

constexpr int BOARD_LCD_DATA15 =
    40;

// =====================================================
// TOUCH - GT911
// =====================================================

constexpr int BOARD_TOUCH_INT =
    4;

constexpr int BOARD_TOUCH_RST_EXIO =
    1;

constexpr int BOARD_TOUCH_WIDTH =
    800;

constexpr int BOARD_TOUCH_HEIGHT =
    480;