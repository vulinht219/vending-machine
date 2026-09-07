#include "AppController.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "SDCardManager.h"
#include "DisplayManager.h"
#include "TouchManager.h"
#include "LVGLManager.h"

#include "HomeScreen.h"

#include "lvgl.h"

#include <exception>


namespace {

constexpr const char* TAG =
    "AppController";

constexpr const char* QUIZ_FILE_PATH =
    "/sdcard/quizzes.jsonl";


void initializeNVS()
{
    esp_err_t err =
        nvs_flash_init();

    if (
        err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND
    ) {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        err =
            nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
}

} // namespace


AppController::AppController()
    : state(AppState::BOOTING)
{
}


void AppController::start()
{
    ESP_LOGI(
        TAG,
        "Candy vending machine starting..."
    );


    state =
        AppState::BOOTING;


    // =================================================
    // NVS
    // =================================================

    initializeNVS();


    // =================================================
    // LCD
    // =================================================

    if (
        !DisplayManager::initialize()
    ) {
        ESP_LOGE(
            TAG,
            "LCD initialization failed."
        );

        return;
    }


    if (
        !DisplayManager::setBacklight(
            true
        )
    ) {
        ESP_LOGE(
            TAG,
            "LCD backlight enable failed."
        );

        return;
    }


    // =================================================
    // GT911 TOUCH
    // =================================================

    if (
        !TouchManager::initialize()
    ) {
        ESP_LOGE(
            TAG,
            "GT911 initialization failed."
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "LCD + GT911 initialized successfully."
    );


    // =================================================
    // LVGL
    // =================================================

    if (
        !LVGLManager::initialize()
    ) {
        ESP_LOGE(
            TAG,
            "LVGL initialization failed."
        );

        return;
    }


    ESP_LOGI(
        TAG,
        "LVGL initialized successfully."
    );


    // =================================================
    // HOME SCREEN
    // =================================================

    HomeScreen::create();


    ESP_LOGI(
        TAG,
        "Real Home screen started."
    );


    // =================================================
    // TEMPORARY LVGL LOOP
    // =================================================
    //
    // For now we keep the LVGL loop here while testing
    // the real Home screen on hardware.
    //

    while (true)
    {
        uint32_t waitMs =
            lv_timer_handler();


        if (
            waitMs < 5
        ) {
            waitMs =
                5;
        }


        if (
            waitMs > 20
        ) {
            waitMs =
                20;
        }


        vTaskDelay(
            pdMS_TO_TICKS(
                waitMs
            )
        );
    }


    // =================================================
    // SD CARD
    // =================================================
    //
    // Not reached while the temporary LVGL loop above
    // is active.
    //

    if (
        !SDCardManager::mount()
    ) {
        state =
            AppState::SD_ERROR;


        ESP_LOGE(
            TAG,
            "SD card initialization failed."
        );


        return;
    }


    // =================================================
    // DATASET
    // =================================================

    try {

        quizSource =
            std::make_unique<
                SDCardQuizSource
            >(
                QUIZ_FILE_PATH
            );


        if (
            quizSource->size() == 0
        ) {
            state =
                AppState::DATASET_ERROR;


            ESP_LOGE(
                TAG,
                "Dataset is empty."
            );


            return;
        }


        game =
            std::make_unique<
                GameManager
            >(
                dispenser,
                *quizSource,
                quizProgressStore,
                gameProgressStore
            );


        state =
            AppState::READY;


        ESP_LOGI(
            TAG,
            "Application READY."
        );


        ESP_LOGI(
            TAG,
            "Quiz count: %u",
            static_cast<unsigned>(
                quizSource->size()
            )
        );
    }

    catch (
        const std::exception& exception
    ) {

        ESP_LOGE(
            TAG,
            "Dataset initialization failed: %s",
            exception.what()
        );


        state =
            AppState::DATASET_ERROR;
    }
}


AppState AppController::getState() const
{
    return state;
}


GameManager* AppController::getGame()
{
    return game.get();
}