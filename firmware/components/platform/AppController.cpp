#include "AppController.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "SDCardManager.h"

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
    // SD CARD
    // =================================================

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