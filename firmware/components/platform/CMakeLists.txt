idf_component_register(
    SRCS
        "NVSQuizProgressStore.cpp"
        "NVSGameProgressStore.cpp"
        "SDCardManager.cpp"
        "SDCardQuizSource.cpp"
        "RealDispenser.cpp"
        "TestQuizSource.cpp"
        "AppController.cpp"
        "NVSSpecialEventProgressStore.cpp"
        "DS3231Clock.cpp"
        "BoardI2CManager.cpp"
        "CH422GManager.cpp"
        "DisplayManager.cpp"
        "TouchManager.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        core
        nvs_flash
        fatfs
        sdmmc
        esp_driver_spi
        esp_driver_i2c
        driver
        esp_lcd
        esp_lcd_touch
        esp_lcd_touch_gt911
)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
    -fexceptions
)