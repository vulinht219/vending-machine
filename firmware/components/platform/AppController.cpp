#include "AppController.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#include "BoardI2CManager.h"
#include "SDCardManager.h"
#include "DisplayManager.h"
#include "TouchManager.h"
#include "LVGLManager.h"
#include "HomeScreen.h"

#include "lvgl.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <memory>

namespace
{

constexpr const char* TAG = "AppController";

constexpr const char* QUIZ_FILE_PATH =
    "/sdcard/quizzes.jsonl";

constexpr uint16_t PCA9685_ADDRESS = 0x40;
constexpr uint16_t DS3231_ADDRESS = 0x68;

constexpr uint8_t RTC_STATUS_REGISTER = 0x0F;

i2c_master_dev_handle_t rtcDevice = nullptr;

bool rtcReady = false;

// =====================================================
// IR BREAK-BEAM SENSOR
// =====================================================

// Waveshare Sensor AD connector.
//
// Receiver yellow OUT -> AD / GPIO6.
// Receiver and transmitter powered from 3.3 V.
//
// Adafruit 2168:
// HIGH = beam clear
// LOW  = beam broken
//
// SENSOR TEST ONLY:
// No servo control is performed here.

constexpr gpio_num_t SENSOR_GPIO = GPIO_NUM_6;

constexpr uint32_t SENSOR_POLL_MS = 20;

constexpr int SENSOR_STABLE_SAMPLES = 3;

bool sensorReady = false;

// =====================================================
// NVS
// =====================================================

void initializeNVS()
{
    esp_err_t err = nvs_flash_init();

    if (
        err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND
    )
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
}

// =====================================================
// RTC HELPERS
// =====================================================

uint8_t toBCD(int value)
{
    return static_cast<uint8_t>(
        ((value / 10) << 4) |
        (value % 10)
    );
}

int fromBCD(uint8_t value)
{
    return ((value >> 4) * 10) +
           (value & 0x0F);
}

bool validBCD(uint8_t value)
{
    return (value & 0x0F) <= 9 &&
           ((value >> 4) & 0x0F) <= 9;
}

bool leapYear(int year)
{
    return year % 400 == 0 ||
           (year % 4 == 0 && year % 100 != 0);
}

int daysInMonth(int year, int month)
{
    static const int days[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12)
    {
        return 0;
    }

    if (month == 2 && leapYear(year))
    {
        return 29;
    }

    return days[month - 1];
}

// =====================================================
// RTC I2C
// =====================================================

bool rtcRead(
    uint8_t reg,
    uint8_t* data,
    size_t length
)
{
    if (!rtcReady || rtcDevice == nullptr)
    {
        return false;
    }

    esp_err_t result =
        i2c_master_transmit_receive(
            rtcDevice,
            &reg,
            1,
            data,
            length,
            100
        );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "RTC read 0x%02X failed: %s",
            reg,
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

bool rtcWrite(
    const uint8_t* data,
    size_t length
)
{
    if (!rtcReady || rtcDevice == nullptr)
    {
        return false;
    }

    esp_err_t result =
        i2c_master_transmit(
            rtcDevice,
            data,
            length,
            100
        );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "RTC write failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

// =====================================================
// READ RTC
// =====================================================

bool readDS3231()
{
    uint8_t status = 0;

    if (!rtcRead(
            RTC_STATUS_REGISTER,
            &status,
            1
        ))
    {
        ESP_LOGE(
            TAG,
            "DS3231 READ FAIL: status"
        );

        return false;
    }

    uint8_t data[7] = {};

    if (!rtcRead(0x00, data, sizeof(data)))
    {
        ESP_LOGE(
            TAG,
            "DS3231 READ FAIL: time"
        );

        return false;
    }

    const uint8_t rawSecond = data[0] & 0x7F;
    const uint8_t rawMinute = data[1] & 0x7F;
    const uint8_t rawDay = data[4] & 0x3F;
    const uint8_t rawMonth = data[5] & 0x1F;

    if (
        !validBCD(rawSecond) ||
        !validBCD(rawMinute) ||
        !validBCD(rawDay) ||
        !validBCD(rawMonth) ||
        !validBCD(data[6])
    )
    {
        ESP_LOGE(
            TAG,
            "DS3231 READ FAIL: invalid BCD"
        );

        return false;
    }

    const int second = fromBCD(rawSecond);
    const int minute = fromBCD(rawMinute);
    const int day = fromBCD(rawDay);
    const int month = fromBCD(rawMonth);

    int hour = 0;

    if (data[2] & 0x40)
    {
        const uint8_t rawHour = data[2] & 0x1F;

        if (!validBCD(rawHour))
        {
            return false;
        }

        const int hour12 = fromBCD(rawHour);

        if (hour12 < 1 || hour12 > 12)
        {
            return false;
        }

        hour = hour12 % 12;

        if (data[2] & 0x20)
        {
            hour += 12;
        }
    }
    else
    {
        const uint8_t rawHour = data[2] & 0x3F;

        if (!validBCD(rawHour))
        {
            return false;
        }

        hour = fromBCD(rawHour);
    }

    const int year =
        2000 +
        fromBCD(data[6]) +
        ((data[5] & 0x80) ? 100 : 0);

    if (
        second > 59 ||
        minute > 59 ||
        hour > 23 ||
        month < 1 ||
        month > 12 ||
        day < 1 ||
        day > daysInMonth(year, month)
    )
    {
        ESP_LOGE(
            TAG,
            "DS3231 READ FAIL: invalid date/time"
        );

        return false;
    }

    ESP_LOGI(
        TAG,
        "DS3231 status=0x%02X, OSF=%d",
        status,
        (status & 0x80) ? 1 : 0
    );

    ESP_LOGI(
        TAG,
        "DS3231 READ PASS: %04d-%02d-%02d %02d:%02d:%02d UTC",
        year,
        month,
        day,
        hour,
        minute,
        second
    );

    if (status & 0x80)
    {
        ESP_LOGW(
            TAG,
            "DS3231 TIME NOT TRUSTED: OSF=1"
        );
    }

    return true;
}

// =====================================================
// SET RTC FROM EXPLICIT UTC COMMAND
// =====================================================

bool setDS3231(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second,
    int weekday
)
{
    if (
        year < 2000 ||
        year > 2099 ||
        month < 1 ||
        month > 12 ||
        day < 1 ||
        day > daysInMonth(year, month) ||
        hour < 0 ||
        hour > 23 ||
        minute < 0 ||
        minute > 59 ||
        second < 0 ||
        second > 59 ||
        weekday < 1 ||
        weekday > 7
    )
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAIL: invalid date/time"
        );

        return false;
    }

    // Seconds, minutes, 24-hour clock,
    // weekday, date, month and year.

    const uint8_t payload[8] = {
        0x00,
        toBCD(second),
        toBCD(minute),
        toBCD(hour),
        toBCD(weekday),
        toBCD(day),
        toBCD(month),
        toBCD(year - 2000)
    };

    if (!rtcWrite(payload, sizeof(payload)))
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAIL: could not write time"
        );

        return false;
    }

    uint8_t status = 0;

    if (!rtcRead(
            RTC_STATUS_REGISTER,
            &status,
            1
        ))
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAIL: could not read status"
        );

        return false;
    }

    // Clear only OSF (bit 7).
    // Preserve the other status bits.

    const uint8_t statusPayload[2] = {
        RTC_STATUS_REGISTER,
        static_cast<uint8_t>(
            status & static_cast<uint8_t>(~0x80)
        )
    };

    if (!rtcWrite(
            statusPayload,
            sizeof(statusPayload)
        ))
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAIL: could not clear OSF"
        );

        return false;
    }

    uint8_t verifiedStatus = 0;

    if (
        !rtcRead(
            RTC_STATUS_REGISTER,
            &verifiedStatus,
            1
        ) ||
        (verifiedStatus & 0x80)
    )
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAIL: OSF still set"
        );

        return false;
    }

    ESP_LOGI(
        TAG,
        "RTC SET: write completed; reading back"
    );

    return readDS3231();
}

// =====================================================
// UART COMMAND PROCESSOR
//
// Accepted format:
// SET YYYY-MM-DD HH:MM:SS W
//
// W = weekday 1..7, Monday = 1.
//
// Only a received SET command writes time.
// Boot does not automatically change RTC.
// =====================================================

void processRTCCommand(const char* command)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int weekday = 0;
    int consumed = 0;

    const int matched = std::sscanf(
        command,
        "SET %d-%d-%d %d:%d:%d %d %n",
        &year,
        &month,
        &day,
        &hour,
        &minute,
        &second,
        &weekday,
        &consumed
    );

    if (
        matched != 7 ||
        command[consumed] != '\0'
    )
    {
        ESP_LOGW(
            TAG,
            "RTC COMMAND REJECTED. Expected: SET YYYY-MM-DD HH:MM:SS W"
        );

        return;
    }

    if (setDS3231(
            year,
            month,
            day,
            hour,
            minute,
            second,
            weekday
        ))
    {
        ESP_LOGI(
            TAG,
            "RTC SET PASS"
        );
    }
    else
    {
        ESP_LOGE(
            TAG,
            "RTC SET FAILED"
        );
    }
}

// =====================================================
// UART RX TASK
// =====================================================

void rtcSerialTask(void* argument)
{
    (void)argument;

    char command[80] = {};
    size_t position = 0;

    while (true)
    {
        uint8_t character = 0;

        const int count = uart_read_bytes(
            UART_NUM_0,
            &character,
            1,
            pdMS_TO_TICKS(100)
        );

        if (count <= 0)
        {
            continue;
        }

        if (
            character == '\r' ||
            character == '\n'
        )
        {
            if (position > 0)
            {
                command[position] = '\0';

                processRTCCommand(command);

                position = 0;
                command[0] = '\0';
            }

            continue;
        }

        if (position < sizeof(command) - 1)
        {
            command[position++] =
                static_cast<char>(character);
        }
        else
        {
            position = 0;

            ESP_LOGW(
                TAG,
                "RTC command too long; discarded"
            );
        }
    }
}

// =====================================================
// START UART RECEIVER
// =====================================================

void startRTCSerialReceiver()
{
    // Existing console is configured for UART0.
    // Do not change its pins or baud rate.

    if (!uart_is_driver_installed(UART_NUM_0))
    {
        esp_err_t result =
            uart_driver_install(
                UART_NUM_0,
                1024,
                0,
                0,
                nullptr,
                0
            );

        if (result != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "RTC SERIAL FAIL: %s",
                esp_err_to_name(result)
            );

            return;
        }
    }

    BaseType_t result = xTaskCreate(
        rtcSerialTask,
        "rtc_serial",
        4096,
        nullptr,
        5,
        nullptr
    );

    if (result != pdPASS)
    {
        ESP_LOGE(
            TAG,
            "RTC SERIAL FAIL: task creation"
        );

        return;
    }

    ESP_LOGI(
        TAG,
        "RTC SERIAL READY: SET YYYY-MM-DD HH:MM:SS W"
    );
}

// =====================================================
// SENSOR: LOG STATE
// =====================================================

void logSensorState(int level)
{
    if (level == 1)
    {
        ESP_LOGI(
            "BreakBeam",
            "BEAM CLEAR: GPIO6=HIGH (1)"
        );
    }
    else
    {
        ESP_LOGW(
            "BreakBeam",
            "BEAM BROKEN: GPIO6=LOW (0)"
        );
    }
}

// =====================================================
// SENSOR: MONITOR TASK
// =====================================================

void sensorMonitorTask(void* argument)
{
    (void)argument;

    // Allow sensor and GPIO input to settle.

    vTaskDelay(pdMS_TO_TICKS(100));

    int stableLevel =
        gpio_get_level(SENSOR_GPIO);

    int candidateLevel = stableLevel;

    int candidateCount = 0;

    ESP_LOGI(
        "BreakBeam",
        "SENSOR MONITOR STARTED: GPIO6, pull-up enabled"
    );

    logSensorState(stableLevel);

    while (true)
    {
        vTaskDelay(
            pdMS_TO_TICKS(SENSOR_POLL_MS)
        );

        const int rawLevel =
            gpio_get_level(SENSOR_GPIO);

        // Ignore readings identical to the
        // currently confirmed stable state.

        if (rawLevel == stableLevel)
        {
            candidateLevel = stableLevel;
            candidateCount = 0;
            continue;
        }

        // A different state must persist across
        // several consecutive samples.

        if (rawLevel != candidateLevel)
        {
            candidateLevel = rawLevel;
            candidateCount = 1;
        }
        else
        {
            ++candidateCount;
        }

        if (
            candidateCount >=
            SENSOR_STABLE_SAMPLES
        )
        {
            stableLevel = candidateLevel;
            candidateCount = 0;

            logSensorState(stableLevel);
        }
    }
}

// =====================================================
// SENSOR: INITIALIZATION
// =====================================================

bool initializeBreakBeamSensor()
{
    if (sensorReady)
    {
        return true;
    }

    gpio_config_t config = {};

    config.pin_bit_mask =
        (1ULL << SENSOR_GPIO);

    config.mode = GPIO_MODE_INPUT;

    config.pull_up_en =
        GPIO_PULLUP_ENABLE;

    config.pull_down_en =
        GPIO_PULLDOWN_DISABLE;

    config.intr_type =
        GPIO_INTR_DISABLE;

    const esp_err_t result =
        gpio_config(&config);

    if (result != ESP_OK)
    {
        ESP_LOGE(
            "BreakBeam",
            "GPIO6 initialization failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    ESP_LOGI(
        "BreakBeam",
        "GPIO6 configured: INPUT + PULL-UP"
    );

    const BaseType_t taskResult =
        xTaskCreate(
            sensorMonitorTask,
            "breakbeam_test",
            3072,
            nullptr,
            4,
            nullptr
        );

    if (taskResult != pdPASS)
    {
        ESP_LOGE(
            "BreakBeam",
            "Sensor monitor task creation FAILED"
        );

        return false;
    }

    sensorReady = true;

    ESP_LOGI(
        "BreakBeam",
        "SENSOR TEST READY: servo control NOT connected"
    );

    return true;
}

} // namespace

// =====================================================
// CONSTRUCTOR
// =====================================================

AppController::AppController()
    : state(AppState::BOOTING)
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

    state = AppState::BOOTING;

    initializeNVS();

    // =================================================
    // LCD
    // =================================================

    if (!DisplayManager::initialize())
    {
        ESP_LOGE(
            TAG,
            "LCD initialization failed."
        );

        return;
    }

    if (!DisplayManager::setBacklight(true))
    {
        ESP_LOGE(
            TAG,
            "LCD backlight enable failed."
        );

        return;
    }

    // =================================================
    // TOUCH
    // =================================================

    if (!TouchManager::initialize())
    {
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
    // SHARED I2C BUS
    // =================================================

    i2c_master_bus_handle_t i2cBus =
        BoardI2CManager::getBus();

    // =================================================
    // PCA9685
    // =================================================

    if (i2cBus == nullptr)
    {
        ESP_LOGE(
            TAG,
            "PCA9685 TEST FAIL: I2C bus is null"
        );
    }
    else
    {
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
    // DS3231
    // =================================================

    if (i2cBus == nullptr)
    {
        ESP_LOGE(
            TAG,
            "DS3231 TEST FAIL: I2C bus is null"
        );
    }
    else
    {
        esp_err_t probeResult =
            i2c_master_probe(
                i2cBus,
                DS3231_ADDRESS,
                100
            );

        if (probeResult == ESP_OK)
        {
            ESP_LOGI(
                TAG,
                "DS3231 TEST PASS: detected at 0x68"
            );

            i2c_device_config_t config = {};

            config.dev_addr_length =
                I2C_ADDR_BIT_LEN_7;

            config.device_address =
                DS3231_ADDRESS;

            config.scl_speed_hz =
                100000;

            esp_err_t result =
                i2c_master_bus_add_device(
                    i2cBus,
                    &config,
                    &rtcDevice
                );

            if (result == ESP_OK)
            {
                rtcReady = true;

                readDS3231();

                startRTCSerialReceiver();
            }
            else
            {
                ESP_LOGE(
                    TAG,
                    "DS3231 device registration FAIL: %s",
                    esp_err_to_name(result)
                );
            }
        }
        else
        {
            ESP_LOGE(
                TAG,
                "DS3231 TEST FAIL: %s",
                esp_err_to_name(probeResult)
            );
        }
    }

    // =================================================
    // LVGL
    // =================================================

    if (!LVGLManager::initialize())
    {
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

    if (!SDCardManager::mount())
    {
        state = AppState::SD_ERROR;

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

    try
    {
        quizSource =
            std::make_unique<SDCardQuizSource>(
                QUIZ_FILE_PATH
            );

        if (quizSource->size() == 0)
        {
            state = AppState::DATASET_ERROR;

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
            std::make_unique<GameManager>(
                dispenser,
                *quizSource,
                quizProgressStore,
                gameProgressStore
            );

        state = AppState::READY;

        ESP_LOGI(
            TAG,
            "Application READY."
        );
    }
    catch (const std::exception& exception)
    {
        ESP_LOGE(
            TAG,
            "Dataset initialization failed: %s",
            exception.what()
        );

        state = AppState::DATASET_ERROR;

        return;
    }

    // =================================================
    // HOME SCREEN
    // =================================================

    HomeScreen::create(*game);

    ESP_LOGI(
        TAG,
        "Home screen started."
    );

    // =================================================
    // TEMPORARY SERVO TEST / CALIBRATION BUTTON
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
        lv_label_create(servoTestButton);

    lv_label_set_text(
        servoTestLabel,
        "TEST SERVO"
    );

    lv_obj_center(servoTestLabel);

    lv_obj_add_event_cb(
        servoTestButton,

        [](lv_event_t* event)
        {
            auto* motor =
                static_cast<RealDispenser*>(
                    lv_event_get_user_data(event)
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
    // IR BREAK-BEAM SENSOR TEST
    // =================================================

    ESP_LOGI(
        TAG,
        "Starting IR break-beam sensor test..."
    );

    if (!initializeBreakBeamSensor())
    {
        ESP_LOGE(
            TAG,
            "IR SENSOR TEST INITIALIZATION FAILED"
        );
    }
    else
    {
        ESP_LOGI(
            TAG,
            "IR SENSOR TEST INITIALIZATION PASS"
        );
    }

    // =================================================
    // LVGL MAIN LOOP
    // =================================================

    while (true)
    {
        uint32_t waitMs =
            lv_timer_handler();

        if (waitMs < 5)
        {
            waitMs = 5;
        }

        if (waitMs > 20)
        {
            waitMs = 20;
        }

        vTaskDelay(
            pdMS_TO_TICKS(waitMs)
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