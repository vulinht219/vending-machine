#pragma once

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>

namespace
{

constexpr const char* BEAM_TAG = "BeamSensor";
constexpr gpio_num_t BEAM_GPIO = GPIO_NUM_6;

// Three identical readings, 20 ms apart.
constexpr int BEAM_SAMPLE_MS = 20;
constexpr int BEAM_STABLE_SAMPLES = 3;

void beamSensorTask(void*)
{
    int stableLevel = gpio_get_level(BEAM_GPIO);
    int candidateLevel = stableLevel;
    int candidateCount = 0;

    ESP_LOGI(
        BEAM_TAG,
        "SENSOR READY: GPIO6 raw=%d",
        stableLevel
    );

    ESP_LOGI(
        BEAM_TAG,
        "Keep beam clear, then block it with a snack"
    );

    while (true)
    {
        const int currentLevel = gpio_get_level(BEAM_GPIO);

        if (currentLevel != candidateLevel)
        {
            candidateLevel = currentLevel;
            candidateCount = 1;
        }
        else if (candidateCount < BEAM_STABLE_SAMPLES)
        {
            ++candidateCount;
        }

        if (
            candidateCount >= BEAM_STABLE_SAMPLES &&
            candidateLevel != stableLevel
        )
        {
            stableLevel = candidateLevel;

            ESP_LOGI(
                BEAM_TAG,
                "SENSOR STATE CHANGED: GPIO6 raw=%d",
                stableLevel
            );
        }

        vTaskDelay(pdMS_TO_TICKS(BEAM_SAMPLE_MS));
    }
}

bool startBeamSensorTest()
{
    static bool started = false;

    if (started)
    {
        return true;
    }

    gpio_config_t config = {};

    config.pin_bit_mask = (1ULL << GPIO_NUM_6);
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = GPIO_PULLUP_ENABLE;
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.intr_type = GPIO_INTR_DISABLE;

    const esp_err_t result = gpio_config(&config);

    if (result != ESP_OK)
    {
        ESP_LOGE(
            BEAM_TAG,
            "GPIO6 initialization failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    const BaseType_t taskResult = xTaskCreate(
        beamSensorTask,
        "beam_sensor_test",
        3072,
        nullptr,
        4,
        nullptr
    );

    if (taskResult != pdPASS)
    {
        ESP_LOGE(
            BEAM_TAG,
            "Sensor task creation failed"
        );

        return false;
    }

    started = true;

    ESP_LOGI(
        BEAM_TAG,
        "GPIO6 input initialized with 3.3V pull-up"
    );

    return true;
}

} // namespace