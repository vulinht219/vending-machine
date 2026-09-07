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


    ESP_ERROR_CHECK(
        err
    );
}

} // namespace


AppController::AppController()
    : state(
        AppState::BOOTING
    )
{
}


// =====================================================
// START
// =====================================================

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
    // TOUCH
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
    // SD CARD
    // =================================================

    ESP_LOGI(
        TAG,
        "Mounting SD card..."
    );


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


    ESP_LOGI(
        TAG,
        "SD card mounted successfully."
    );


    // =================================================
    // DATASET + GAME
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
                "Quiz dataset is empty."
            );


            return;
        }


        ESP_LOGI(
            TAG,
            "Quiz count: %u",
            static_cast<unsigned>(
                quizSource->size()
            )
        );


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


        return;
    }


    // =================================================
    // REAL HOME SCREEN
    // =================================================

    HomeScreen::create(
        *game
    );


    ESP_LOGI(
        TAG,
        "Home screen started."
    );


    // =================================================
    // LVGL MAIN LOOP
    // =================================================

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
}


// =====================================================
// STATE
// =====================================================

AppState AppController::getState() const
{
    return state;
}


// =====================================================
// GAME
// =====================================================

GameManager* AppController::getGame()
{
    return game.get();
}