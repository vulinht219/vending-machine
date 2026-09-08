idf_component_register(
    SRCS
        "main.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        fatfs
        sdmmc
        esp_driver_spi
        esp_driver_i2c
        esp_timer
)