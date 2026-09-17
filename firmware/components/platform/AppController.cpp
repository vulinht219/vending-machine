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

#include "BoardI2CManager.h"
#include "driver/i2c_master.h"

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
    // PCA9685 I2C CONNECTION TEST
    // =================================================

    i2c_master_bus_handle_t i2cBus =
        BoardI2CManager::getBus();

    if (i2cBus == nullptr)
    {
        ESP_LOGE(
            TAG,
            "PCA9685 TEST FAIL: I2C bus is null"
        );
    }
    else
    {
        constexpr uint16_t PCA9685_ADDRESS =
            0x40;

        esp_err_t probeResult =
            i2c_master_probe(
                i2cBus,
                PCA9685_ADDRESS,
                100
            );

        if (probeResult == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "PCA9685 TEST PASS: detected at 0x40"
            );

            if (!dispenser.initializeServo0Neutral())
            {
                ESP_LOGE(
                    TAG,
                    "PCA9685 CH0 neutral initialization FAILED"
                );
            }
            else
            {
                ESP_LOGI(
                    TAG,
                    "PCA9685 CH0 neutral initialization PASS"
                );
            }
        }
        else
        {
            ESP_LOGW(
                TAG,
                "PCA9685 TEST FAIL: %s",
                esp_err_to_name(probeResult)
            );
        }
    }

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
    // TEMPORARY SERVO TEST BUTTON
    // =================================================

    lv_obj_t* servoTestButton =
        lv_button_create(
            lv_screen_active()
        );

    lv_obj_set_size(
        servoTestButton,
        180,
        55
    );

    lv_obj_align(
        servoTestButton,
        LV_ALIGN_TOP_RIGHT,
        -10,
        10
    );

    lv_obj_t* servoTestLabel =
        lv_label_create(
            servoTestButton
        );

    lv_label_set_text(
        servoTestLabel,
        "TEST SERVO"
    );

    lv_obj_center(
        servoTestLabel
    );

    lv_obj_add_event_cb(
        servoTestButton,

        [](lv_event_t* event)
        {
            auto* motor =
                static_cast<RealDispenser*>(
                    lv_event_get_user_data(
                        event
                    )
                );

            if (motor == nullptr)
            {
                ESP_LOGE(
                    "ServoTest",
                    "Dispenser pointer is null"
                );

                return;
            }

            if (!motor->testServo0Once())
            {
                ESP_LOGE(
                    "ServoTest",
                    "One-shot servo test FAILED"
                );
            }
            else
            {
                ESP_LOGI(
                    "ServoTest",
                    "One-shot servo test PASS"
                );
            }
        },

        LV_EVENT_CLICKED,
        &dispenser
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