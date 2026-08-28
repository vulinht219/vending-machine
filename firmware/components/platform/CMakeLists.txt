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
)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
    -fexceptions
)