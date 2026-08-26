idf_component_register(
    SRCS
        "NVSQuizProgressStore.cpp"
        "NVSGameProgressStore.cpp"
        "SDCardManager.cpp"
        "SDCardQuizSource.cpp"
        "RealDispenser.cpp"
        "TestQuizSource.cpp"
        "AppController.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        core
        nvs_flash
        fatfs
        sdmmc
        esp_driver_spi
        driver
)

target_compile_options(
    ${COMPONENT_LIB}
    PRIVATE
    -fexceptions
)