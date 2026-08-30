#include "AppController.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "SDCardManager.h"
#include "DisplayManager.h"
#include "TouchManager.h"

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
    // TEMPORARY LCD HARDWARE SELF TEST
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
        !DisplayManager::runSelfTest()
    ) {
        ESP_LOGE(
            TAG,
            "LCD self-test failed."
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
    // TEMPORARY GT911 TOUCH SELF TEST
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
    // TEMPORARY BLOCKING TOUCH TEST
    // =================================================
    //
    // This intentionally blocks here.
    //
    // While testing hardware:
    //
    // - LCD should show RGB test bars
    // - touching the display should print:
    //
    //   TOUCH x=123 y=456
    //
    // Remove this call after hardware validation.
    //

    TouchManager::runSelfTest();


    // =================================================
    // SD CARD
    // =================================================
    //
    // NOTE:
    //
    // During the temporary touch self-test above,
    // execution will NOT reach this section because
    // runSelfTest() contains a blocking loop.
    //
    // This code remains here so we can simply remove
    // the self-test call later without rebuilding the
    // production startup architecture.
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