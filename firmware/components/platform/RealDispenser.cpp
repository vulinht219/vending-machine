#include "RealDispenser.h"
#include "BoardI2CManager.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs.h"
#include "lvgl.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace
{

constexpr const char* TAG = "RealDispenser";

// =====================================================
// PCA9685
// =====================================================

constexpr uint8_t PCA9685_ADDRESS = 0x40;
constexpr uint8_t REG_MODE1 = 0x00;
constexpr uint8_t REG_MODE2 = 0x01;
constexpr uint8_t REG_LED0_ON_L = 0x06;
constexpr uint8_t REG_PRESCALE = 0xFE;
constexpr uint8_t SERVO_PRESCALE = 121;

constexpr int CHANNEL_COUNT = 16;
constexpr int SERVO_COUNT = 6;

// =====================================================
// SENSOR
// =====================================================

// Receiver OUT -> GPIO6.
// HIGH = clear.
// LOW = beam interrupted.
//
// IMPORTANT: GPIO6 must receive 3.3 V logic, not 5 V.

constexpr gpio_num_t SENSOR_GPIO = GPIO_NUM_6;
constexpr int64_t SENSOR_CLEAR_US = 20000;
constexpr int SENSOR_LOW_SAMPLES = 2;

// =====================================================
// LIMITS
// =====================================================

constexpr uint16_t MIN_PULSE = 280;
constexpr uint16_t MAX_PULSE = 325;

constexpr uint32_t MIN_DURATION_MS = 100;
constexpr uint32_t MAX_DURATION_MS = 2500;
constexpr uint32_t DURATION_STEP_MS = 250;

// =====================================================
// NVS
// =====================================================

constexpr const char* NVS_NAMESPACE = "servo_cal";
constexpr const char* NVS_KEY = "profiles";

constexpr uint32_t CONFIG_MAGIC = 0x53455256;
constexpr uint32_t CONFIG_VERSION = 1;

// =====================================================
// PROFILES
// =====================================================

struct ServoProfile
{
    uint16_t pulse;
    uint32_t durationMs;
};

// Measured defaults. Saved NVS profiles override these.

ServoProfile profiles[SERVO_COUNT] = {
    {307, 2100},
    {307, 2100},
    {307, 2100},
    {307, 1850},
    {307, 2100},
    {307, 1600}
};

struct StoredServoConfig
{
    uint32_t magic;
    uint32_t version;
    ServoProfile profiles[SERVO_COUNT];
};

bool profilesDirty = true;

// =====================================================
// HARDWARE STATE
// =====================================================

i2c_master_dev_handle_t device = nullptr;

bool initialized = false;

std::atomic<bool> faultLatched{false};
std::atomic<bool> cycleRunning{false};
std::atomic<bool> stopRequested{false};

// Calibration status:
// 0 ready, 1 running, 2 complete, 3 stopped, 4 fault.

std::atomic<int> cycleStatus{0};

std::atomic<DispenseStatus> dispenseStatus{
    DispenseStatus::Idle
};

std::atomic<int> lastDispenseSlot{0};

struct TestConfiguration
{
    int channel;
    uint16_t pulse;
    uint32_t durationMs;
};

TestConfiguration activeTest = {
    0, 307, 2100
};

TestConfiguration activeDispense = {
    0, 307, 2100
};

// =====================================================
// CALIBRATION UI STATE
// =====================================================

int selectedChannel = 0;

lv_obj_t* calibrationPanel = nullptr;

lv_obj_t* channelLabel = nullptr;
lv_obj_t* pulseLabel = nullptr;
lv_obj_t* durationLabel = nullptr;
lv_obj_t* statusLabel = nullptr;
lv_obj_t* saveStatusLabel = nullptr;

lv_timer_t* calibrationTimer = nullptr;

// =====================================================
// VALIDATION
// =====================================================

bool validChannel(int channel)
{
    return channel >= 0 &&
           channel < CHANNEL_COUNT;
}

bool validServoChannel(int channel)
{
    return channel >= 0 &&
           channel < SERVO_COUNT;
}

bool validProfile(const ServoProfile& profile)
{
    return
        profile.pulse >= MIN_PULSE &&
        profile.pulse <= MAX_PULSE &&
        profile.durationMs >= MIN_DURATION_MS &&
        profile.durationMs <= MAX_DURATION_MS;
}

// =====================================================
// LOAD PROFILES
// =====================================================

bool loadProfiles()
{
    nvs_handle_t handle;

    esp_err_t result = nvs_open(
        NVS_NAMESPACE,
        NVS_READONLY,
        &handle
    );

    if (result == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(
            TAG,
            "No saved profiles; using defaults"
        );

        profilesDirty = true;
        return true;
    }

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS open failed: %s",
            esp_err_to_name(result)
        );

        profilesDirty = true;
        return false;
    }

    StoredServoConfig stored = {};
    size_t size = sizeof(stored);

    result = nvs_get_blob(
        handle,
        NVS_KEY,
        &stored,
        &size
    );

    nvs_close(handle);

    if (result == ESP_ERR_NVS_NOT_FOUND)
    {
        profilesDirty = true;
        return true;
    }

    if (
        result != ESP_OK ||
        size != sizeof(StoredServoConfig)
    )
    {
        ESP_LOGE(TAG, "NVS profile read failed");
        profilesDirty = true;
        return false;
    }

    if (
        stored.magic != CONFIG_MAGIC ||
        stored.version != CONFIG_VERSION
    )
    {
        ESP_LOGE(TAG, "Incompatible NVS profile");
        profilesDirty = true;
        return false;
    }

    for (int channel = 0;
         channel < SERVO_COUNT;
         ++channel)
    {
        if (!validProfile(stored.profiles[channel]))
        {
            ESP_LOGE(
                TAG,
                "Invalid saved profile CH%d",
                channel
            );

            profilesDirty = true;
            return false;
        }
    }

    for (int channel = 0;
         channel < SERVO_COUNT;
         ++channel)
    {
        profiles[channel] = stored.profiles[channel];
    }

    profilesDirty = false;

    ESP_LOGI(TAG, "Loaded all six servo profiles");
    return true;
}

// =====================================================
// SAVE PROFILES
// =====================================================

bool saveProfiles()
{
    if (
        cycleRunning.load() ||
        faultLatched.load()
    )
    {
        ESP_LOGW(TAG, "Cannot save while running/fault");
        return false;
    }

    StoredServoConfig stored = {};

    stored.magic = CONFIG_MAGIC;
    stored.version = CONFIG_VERSION;

    for (int channel = 0;
         channel < SERVO_COUNT;
         ++channel)
    {
        if (!validProfile(profiles[channel]))
        {
            ESP_LOGE(
                TAG,
                "Invalid profile CH%d",
                channel
            );

            return false;
        }

        stored.profiles[channel] = profiles[channel];
    }

    nvs_handle_t handle;

    esp_err_t result = nvs_open(
        NVS_NAMESPACE,
        NVS_READWRITE,
        &handle
    );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS write open failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    result = nvs_set_blob(
        handle,
        NVS_KEY,
        &stored,
        sizeof(stored)
    );

    if (result == ESP_OK)
    {
        result = nvs_commit(handle);
    }

    nvs_close(handle);

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "NVS save failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    profilesDirty = false;

    ESP_LOGI(TAG, "Saved all six servo profiles");

    for (int channel = 0;
         channel < SERVO_COUNT;
         ++channel)
    {
        ESP_LOGI(
            TAG,
            "CH%d PWM=%u duration=%u ms",
            channel,
            static_cast<unsigned>(profiles[channel].pulse),
            static_cast<unsigned>(profiles[channel].durationMs)
        );
    }

    return true;
}

// =====================================================
// I2C
// =====================================================

bool writeBytes(
    const uint8_t* bytes,
    size_t size
)
{
    if (
        device == nullptr ||
        bytes == nullptr ||
        size == 0
    )
    {
        return false;
    }

    const esp_err_t result = i2c_master_transmit(
        device,
        bytes,
        size,
        100
    );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "PCA9685 write failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

bool writeRegister(
    uint8_t reg,
    uint8_t value
)
{
    const uint8_t bytes[2] = {
        reg, value
    };

    return writeBytes(bytes, sizeof(bytes));
}

bool readRegisters(
    uint8_t reg,
    uint8_t* buffer,
    size_t size
)
{
    if (
        device == nullptr ||
        buffer == nullptr ||
        size == 0
    )
    {
        return false;
    }

    const esp_err_t result =
        i2c_master_transmit_receive(
            device,
            &reg,
            1,
            buffer,
            size,
            100
        );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "PCA9685 read failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

// =====================================================
// PWM
// =====================================================

uint8_t channelRegister(int channel)
{
    return static_cast<uint8_t>(
        REG_LED0_ON_L + 4 * channel
    );
}

bool setPulse(
    int channel,
    uint16_t pulse
)
{
    if (
        !validChannel(channel) ||
        pulse > 4095
    )
    {
        return false;
    }

    const uint8_t bytes[5] = {
        channelRegister(channel),
        0x00,
        0x00,
        static_cast<uint8_t>(pulse & 0xFF),
        static_cast<uint8_t>((pulse >> 8) & 0x0F)
    };

    return writeBytes(bytes, sizeof(bytes));
}

bool setFullOff(int channel)
{
    if (!validChannel(channel))
    {
        return false;
    }

    const uint8_t bytes[5] = {
        channelRegister(channel),
        0x00,
        0x00,
        0x00,
        0x10
    };

    return writeBytes(bytes, sizeof(bytes));
}

bool verifyPulse(
    int channel,
    uint16_t pulse
)
{
    if (!validChannel(channel))
    {
        return false;
    }

    uint8_t pwm[4] = {};

    if (!readRegisters(
            channelRegister(channel),
            pwm,
            sizeof(pwm)
        ))
    {
        return false;
    }

    return
        pwm[0] == 0x00 &&
        pwm[1] == 0x00 &&
        pwm[2] == static_cast<uint8_t>(pulse & 0xFF) &&
        pwm[3] == static_cast<uint8_t>((pulse >> 8) & 0x0F);
}

bool verifyFullOff(int channel)
{
    if (!validChannel(channel))
    {
        return false;
    }

    uint8_t pwm[4] = {};

    if (!readRegisters(
            channelRegister(channel),
            pwm,
            sizeof(pwm)
        ))
    {
        return false;
    }

    return (pwm[3] & 0x10) != 0;
}

bool fullOffChecked(int channel)
{
    return
        setFullOff(channel) &&
        verifyFullOff(channel);
}

// =====================================================
// FAULT HANDLING
// =====================================================

void latchFault(const char* stage)
{
    faultLatched.store(true);
    cycleStatus.store(4);
    dispenseStatus.store(DispenseStatus::Fault);

    ESP_LOGE(TAG, "FAULT at %s", stage);

    // Best effort. If I2C fails, use the physical power switch.

    for (int channel = 0;
         channel < CHANNEL_COUNT;
         ++channel)
    {
        setFullOff(channel);
    }

    ESP_LOGE(
        TAG,
        "FAULT LATCHED: TURN SERVO POWER OFF"
    );
}

// =====================================================
// PCA9685 INITIALIZATION
// =====================================================

bool initializePCA9685()
{
    if (faultLatched.load())
    {
        return false;
    }

    if (initialized)
    {
        return true;
    }

    const i2c_master_bus_handle_t bus =
        BoardI2CManager::getBus();

    if (bus == nullptr)
    {
        ESP_LOGE(TAG, "I2C bus is null");
        return false;
    }

    i2c_device_config_t config = {};

    config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    config.device_address = PCA9685_ADDRESS;
    config.scl_speed_hz = 100000;

    const esp_err_t result =
        i2c_master_bus_add_device(
            bus,
            &config,
            &device
        );

    if (result != ESP_OK)
    {
        device = nullptr;

        ESP_LOGE(
            TAG,
            "PCA9685 registration failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    // Sleep + auto-increment, configure 50 Hz.

    if (
        !writeRegister(REG_MODE1, 0x30) ||
        !writeRegister(REG_MODE2, 0x04) ||
        !writeRegister(
            REG_PRESCALE,
            SERVO_PRESCALE
        )
    )
    {
        latchFault("configuration");
        return false;
    }

    // Every channel FULL OFF before waking oscillator.

    for (int channel = 0;
         channel < CHANNEL_COUNT;
         ++channel)
    {
        if (!setFullOff(channel))
        {
            latchFault("startup FULL OFF");
            return false;
        }
    }

    if (!writeRegister(REG_MODE1, 0x20))
    {
        latchFault("oscillator wake");
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2));

    uint8_t mode1 = 0;
    uint8_t mode2 = 0;
    uint8_t prescale = 0;

    if (
        !readRegisters(REG_MODE1, &mode1, 1) ||
        !readRegisters(REG_MODE2, &mode2, 1) ||
        !readRegisters(REG_PRESCALE, &prescale, 1) ||
        mode1 != 0x20 ||
        mode2 != 0x04 ||
        prescale != SERVO_PRESCALE
    )
    {
        latchFault("configuration verification");
        return false;
    }

    for (int channel = 0;
         channel < CHANNEL_COUNT;
         ++channel)
    {
        if (!verifyFullOff(channel))
        {
            latchFault("startup FULL OFF verification");
            return false;
        }
    }

    // Restore calibrated profiles.

    loadProfiles();

    initialized = true;

    ESP_LOGI(TAG, "READY: all 16 channels FULL OFF");
    ESP_LOGI(TAG, "CALIBRATION READY: CH0-CH5");
    ESP_LOGI(TAG, "GAME DISPENSING ENABLED");

    return true;
}

// =====================================================
// CALIBRATION TASK
// =====================================================

void calibrationTask(void*)
{
    const TestConfiguration test = activeTest;

    ESP_LOGI(
        TAG,
        "CALIBRATION START CH%d pulse=%u duration=%u",
        test.channel,
        static_cast<unsigned>(test.pulse),
        static_cast<unsigned>(test.durationMs)
    );

    if (!verifyFullOff(test.channel))
    {
        latchFault("calibration pre-run FULL OFF");
        cycleRunning.store(false);
        vTaskDelete(nullptr);
        return;
    }

    if (
        !setPulse(test.channel, test.pulse) ||
        !verifyPulse(test.channel, test.pulse)
    )
    {
        latchFault("calibration RUN");
        cycleRunning.store(false);
        vTaskDelete(nullptr);
        return;
    }

    const int64_t deadlineUs =
        esp_timer_get_time() +
        static_cast<int64_t>(test.durationMs) * 1000;

    while (
        esp_timer_get_time() < deadlineUs &&
        !stopRequested.load()
    )
    {
        vTaskDelay(1);
    }

    const bool wasStopped = stopRequested.load();

    if (!fullOffChecked(test.channel))
    {
        latchFault("calibration final FULL OFF");
        cycleRunning.store(false);
        vTaskDelete(nullptr);
        return;
    }

    if (wasStopped)
    {
        cycleStatus.store(3);

        ESP_LOGW(
            TAG,
            "CALIBRATION STOPPED CH%d",
            test.channel
        );
    }
    else
    {
        cycleStatus.store(2);

        ESP_LOGI(
            TAG,
            "CALIBRATION COMPLETE CH%d",
            test.channel
        );
    }

    cycleRunning.store(false);
    vTaskDelete(nullptr);
}

// =====================================================
// DISPENSE COMPLETION
// =====================================================

void finishDispense(
    const TestConfiguration& job,
    DispenseStatus result
)
{
    if (!fullOffChecked(job.channel))
    {
        latchFault("dispense final FULL OFF");
    }
    else if (!faultLatched.load())
    {
        dispenseStatus.store(result);

        cycleStatus.store(
            result == DispenseStatus::CandyDetected
                ? 2
                : 3
        );

        ESP_LOGI(
            TAG,
            "DISPENSE END slot=%d CH%d status=%d FULL OFF",
            job.channel + 1,
            job.channel,
            static_cast<int>(result)
        );
    }

    cycleRunning.store(false);
    vTaskDelete(nullptr);
}

// =====================================================
// DISPENSE TASK
// =====================================================

void dispenseTask(void*)
{
    const TestConfiguration job = activeDispense;

    if (!verifyFullOff(job.channel))
    {
        latchFault("dispense pre-run FULL OFF");
        cycleRunning.store(false);
        vTaskDelete(nullptr);
        return;
    }

    // Require beam to be clear before starting.

    const int64_t clearStart =
        esp_timer_get_time();

    while (
        esp_timer_get_time() - clearStart <
        SENSOR_CLEAR_US
    )
    {
        if (stopRequested.load())
        {
            finishDispense(
                job,
                DispenseStatus::Stopped
            );

            return;
        }

        if (gpio_get_level(SENSOR_GPIO) != 1)
        {
            ESP_LOGW(
                TAG,
                "DISPENSE REJECTED: sensor blocked"
            );

            finishDispense(
                job,
                DispenseStatus::SensorBlocked
            );

            return;
        }

        vTaskDelay(1);
    }

    if (stopRequested.load())
    {
        finishDispense(
            job,
            DispenseStatus::Stopped
        );

        return;
    }

    // Maximum rotation time for this slot.

    const int64_t deadlineUs =
        esp_timer_get_time() +
        static_cast<int64_t>(job.durationMs) * 1000;

    if (
        !setPulse(job.channel, job.pulse) ||
        !verifyPulse(job.channel, job.pulse)
    )
    {
        latchFault("dispense motor start");
        cycleRunning.store(false);
        vTaskDelete(nullptr);
        return;
    }

    int consecutiveLow = 0;

    DispenseStatus result =
        DispenseStatus::TimedOut;

    while (esp_timer_get_time() < deadlineUs)
    {
        if (stopRequested.load())
        {
            result = DispenseStatus::Stopped;
            break;
        }

        if (gpio_get_level(SENSOR_GPIO) == 0)
        {
            ++consecutiveLow;

            if (consecutiveLow >= SENSOR_LOW_SAMPLES)
            {
                result = DispenseStatus::CandyDetected;
                break;
            }
        }
        else
        {
            consecutiveLow = 0;
        }

        vTaskDelay(1);
    }

    if (stopRequested.load())
    {
        result = DispenseStatus::Stopped;
    }

    finishDispense(job, result);
}

// =====================================================
// START CALIBRATION
// =====================================================

bool startCalibrationRun()
{
    if (
        !initialized ||
        faultLatched.load()
    )
    {
        ESP_LOGE(TAG, "Calibration rejected: not ready");
        return false;
    }

    bool expected = false;

    if (!cycleRunning.compare_exchange_strong(
            expected,
            true
        ))
    {
        ESP_LOGW(TAG, "Calibration rejected: busy");
        return false;
    }

    if (!validServoChannel(selectedChannel))
    {
        cycleRunning.store(false);
        return false;
    }

    activeTest.channel = selectedChannel;

    activeTest.pulse =
        profiles[selectedChannel].pulse;

    activeTest.durationMs =
        profiles[selectedChannel].durationMs;

    stopRequested.store(false);
    cycleStatus.store(1);

    const BaseType_t result = xTaskCreate(
        calibrationTask,
        "servo_calibration",
        4096,
        nullptr,
        5,
        nullptr
    );

    if (result != pdPASS)
    {
        cycleRunning.store(false);
        cycleStatus.store(4);

        ESP_LOGE(TAG, "Calibration task creation failed");
        return false;
    }

    ESP_LOGI(
        TAG,
        "CALIBRATION ACCEPTED CH%d",
        activeTest.channel
    );

    return true;
}

// =====================================================
// CALIBRATION UI HELPERS
// =====================================================

void refreshCalibrationLabels()
{
    if (calibrationPanel == nullptr)
    {
        return;
    }

    const ServoProfile& selected =
        profiles[selectedChannel];

    char text[96];

    if (channelLabel != nullptr)
    {
        std::snprintf(
            text,
            sizeof(text),
            "CHANNEL: CH%d / SLOT %d",
            selectedChannel,
            selectedChannel + 1
        );

        lv_label_set_text(channelLabel, text);
    }

    if (pulseLabel != nullptr)
    {
        std::snprintf(
            text,
            sizeof(text),
            "PWM: %u",
            static_cast<unsigned>(selected.pulse)
        );

        lv_label_set_text(pulseLabel, text);
    }

    if (durationLabel != nullptr)
    {
        std::snprintf(
            text,
            sizeof(text),
            "DURATION: %u ms",
            static_cast<unsigned>(selected.durationMs)
        );

        lv_label_set_text(durationLabel, text);
    }

    if (statusLabel != nullptr)
    {
        const int status = cycleStatus.load();

        const char* message = "READY";

        switch (status)
        {
            case 0:
                message = "READY";
                break;

            case 1:
                message = "RUNNING - USE STOP";
                break;

            case 2:
                message = "COMPLETE - CHECK SERVO";
                break;

            case 3:
                message = "STOPPED";
                break;

            case 4:
                message = "FAULT - SWITCH POWER OFF";
                break;

            default:
                message = "UNKNOWN";
                break;
        }

        lv_label_set_text(statusLabel, message);
    }

    if (saveStatusLabel != nullptr)
    {
        lv_label_set_text(
            saveStatusLabel,
            profilesDirty
                ? "UNSAVED CHANGES - PRESS SAVE ALL"
                : "ALL 6 PROFILES SAVED"
        );
    }
}

// =====================================================
// BUTTON FACTORY
// =====================================================

lv_obj_t* makeButton(
    lv_obj_t* parent,
    const char* label,
    int x,
    int y,
    int width,
    int height,
    lv_event_cb_t callback
)
{
    lv_obj_t* button =
        lv_button_create(parent);

    lv_obj_set_size(
        button,
        width,
        height
    );

    lv_obj_set_pos(
        button,
        x,
        y
    );

    lv_obj_t* text =
        lv_label_create(button);

    lv_label_set_text(text, label);
    lv_obj_center(text);

    lv_obj_add_event_cb(
        button,
        callback,
        LV_EVENT_CLICKED,
        nullptr
    );

    return button;
}

// =====================================================
// CALIBRATION BUTTON CALLBACKS
// =====================================================

void previousChannel(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    if (selectedChannel > 0)
    {
        --selectedChannel;
    }

    refreshCalibrationLabels();
}

void nextChannel(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    if (selectedChannel < SERVO_COUNT - 1)
    {
        ++selectedChannel;
    }

    refreshCalibrationLabels();
}

void decreasePulse(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    ServoProfile& selected =
        profiles[selectedChannel];

    if (selected.pulse > MIN_PULSE)
    {
        --selected.pulse;
        profilesDirty = true;
    }

    refreshCalibrationLabels();
}

void increasePulse(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    ServoProfile& selected =
        profiles[selectedChannel];

    if (selected.pulse < MAX_PULSE)
    {
        ++selected.pulse;
        profilesDirty = true;
    }

    refreshCalibrationLabels();
}

void decreaseDuration(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    ServoProfile& selected =
        profiles[selectedChannel];

    if (selected.durationMs > MIN_DURATION_MS)
    {
        if (
            selected.durationMs >
            MIN_DURATION_MS + DURATION_STEP_MS
        )
        {
            selected.durationMs -= DURATION_STEP_MS;
        }
        else
        {
            selected.durationMs = MIN_DURATION_MS;
        }

        profilesDirty = true;
    }

    refreshCalibrationLabels();
}

void increaseDuration(lv_event_t*)
{
    if (cycleRunning.load())
    {
        return;
    }

    ServoProfile& selected =
        profiles[selectedChannel];

    if (selected.durationMs < MAX_DURATION_MS)
    {
        if (
            selected.durationMs + DURATION_STEP_MS >
            MAX_DURATION_MS
        )
        {
            selected.durationMs = MAX_DURATION_MS;
        }
        else
        {
            selected.durationMs += DURATION_STEP_MS;
        }

        profilesDirty = true;
    }

    refreshCalibrationLabels();
}

void runButtonClicked(lv_event_t*)
{
    startCalibrationRun();
    refreshCalibrationLabels();
}

void stopButtonClicked(lv_event_t*)
{
    if (cycleRunning.load())
    {
        stopRequested.store(true);
        ESP_LOGW(TAG, "STOP REQUESTED");
    }
    else
    {
        ESP_LOGI(TAG, "No active servo");
    }

    refreshCalibrationLabels();
}

void saveButtonClicked(lv_event_t*)
{
    if (saveProfiles())
    {
        ESP_LOGI(TAG, "SAVE ALL SUCCESS");
    }
    else
    {
        ESP_LOGE(TAG, "SAVE ALL FAILED");
    }

    refreshCalibrationLabels();
}

void closeButtonClicked(lv_event_t*)
{
    if (cycleRunning.load())
    {
        ESP_LOGW(
            TAG,
            "Cannot close while servo is running"
        );

        return;
    }

    if (calibrationTimer != nullptr)
    {
        lv_timer_delete(calibrationTimer);
        calibrationTimer = nullptr;
    }

    if (calibrationPanel != nullptr)
    {
        lv_obj_delete(calibrationPanel);
        calibrationPanel = nullptr;
    }

    channelLabel = nullptr;
    pulseLabel = nullptr;
    durationLabel = nullptr;
    statusLabel = nullptr;
    saveStatusLabel = nullptr;
}

// =====================================================
// CALIBRATION TIMER
// =====================================================

void calibrationTimerCallback(lv_timer_t*)
{
    refreshCalibrationLabels();
}

// =====================================================
// OPEN CALIBRATION PANEL
// =====================================================

bool openCalibrationPanel()
{
    if (
        !initialized ||
        faultLatched.load() ||
        cycleRunning.load()
    )
    {
        return false;
    }

    if (calibrationPanel != nullptr)
    {
        return true;
    }

    calibrationPanel =
        lv_obj_create(lv_screen_active());

    if (calibrationPanel == nullptr)
    {
        return false;
    }

    lv_obj_set_size(
        calibrationPanel,
        450,
        620
    );

    lv_obj_center(calibrationPanel);

    lv_obj_clear_flag(
        calibrationPanel,
        LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_move_foreground(calibrationPanel);

    lv_obj_t* title =
        lv_label_create(calibrationPanel);

    lv_label_set_text(
        title,
        "SERVO CALIBRATION"
    );

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        12
    );

    // Channel.

    channelLabel =
        lv_label_create(calibrationPanel);

    lv_obj_align(
        channelLabel,
        LV_ALIGN_TOP_MID,
        0,
        65
    );

    makeButton(
        calibrationPanel,
        "< CH",
        30, 105, 145, 55,
        previousChannel
    );

    makeButton(
        calibrationPanel,
        "CH >",
        235, 105, 145, 55,
        nextChannel
    );

    // Pulse.

    pulseLabel =
        lv_label_create(calibrationPanel);

    lv_obj_align(
        pulseLabel,
        LV_ALIGN_TOP_MID,
        0,
        185
    );

    makeButton(
        calibrationPanel,
        "PWM -",
        30, 220, 145, 55,
        decreasePulse
    );

    makeButton(
        calibrationPanel,
        "PWM +",
        235, 220, 145, 55,
        increasePulse
    );

    // Duration.

    durationLabel =
        lv_label_create(calibrationPanel);

    lv_obj_align(
        durationLabel,
        LV_ALIGN_TOP_MID,
        0,
        300
    );

    makeButton(
        calibrationPanel,
        "TIME -",
        30, 335, 145, 55,
        decreaseDuration
    );

    makeButton(
        calibrationPanel,
        "TIME +",
        235, 335, 145, 55,
        increaseDuration
    );

    // RUN / STOP.

    makeButton(
        calibrationPanel,
        "RUN",
        30, 415, 145, 55,
        runButtonClicked
    );

    makeButton(
        calibrationPanel,
        "STOP",
        235, 415, 145, 55,
        stopButtonClicked
    );

    // Status.

    statusLabel =
        lv_label_create(calibrationPanel);

    lv_obj_set_width(statusLabel, 405);

    lv_obj_set_style_text_align(
        statusLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        statusLabel,
        LV_ALIGN_TOP_MID,
        0,
        482
    );

    // Save status.

    saveStatusLabel =
        lv_label_create(calibrationPanel);

    lv_obj_set_width(saveStatusLabel, 405);

    lv_obj_set_style_text_align(
        saveStatusLabel,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        saveStatusLabel,
        LV_ALIGN_TOP_MID,
        0,
        515
    );

    // SAVE / CLOSE.

    makeButton(
        calibrationPanel,
        "SAVE ALL",
        30, 545, 145, 45,
        saveButtonClicked
    );

    makeButton(
        calibrationPanel,
        "CLOSE",
        235, 545, 145, 45,
        closeButtonClicked
    );

    refreshCalibrationLabels();

    calibrationTimer = lv_timer_create(
        calibrationTimerCallback,
        100,
        nullptr
    );

    ESP_LOGI(TAG, "Calibration panel opened");
    return true;
}

} // namespace

// =====================================================
// PUBLIC API
// =====================================================

bool RealDispenser::initializeServo0Neutral()
{
    return initializePCA9685();
}

bool RealDispenser::testServo0Once()
{
    return openCalibrationPanel();
}

// =====================================================
// ASYNC MOTOR BACKEND
// =====================================================

// Returns true when a command is ACCEPTED.
// Does not mean a candy has been detected.

bool RealDispenser::startDispense(int slot)
{
    if (
        slot < 1 ||
        slot > SERVO_COUNT ||
        !initialized ||
        faultLatched.load()
    )
    {
        ESP_LOGW(
            TAG,
            "START REJECTED slot=%d",
            slot
        );

        return false;
    }

    if (calibrationPanel != nullptr)
    {
        ESP_LOGW(
            TAG,
            "Close calibration panel first"
        );

        return false;
    }

    const int channel = slot - 1;

    const ServoProfile profile =
        profiles[channel];

    if (!validProfile(profile))
    {
        ESP_LOGE(
            TAG,
            "Invalid profile CH%d",
            channel
        );

        return false;
    }

    // Allow only one active servo.

    bool expected = false;

    if (!cycleRunning.compare_exchange_strong(
            expected,
            true
        ))
    {
        ESP_LOGW(TAG, "Motor is busy");
        return false;
    }

    // Configure the sensor input.

    gpio_config_t sensor = {};

    sensor.pin_bit_mask =
        (1ULL << SENSOR_GPIO);

    sensor.mode = GPIO_MODE_INPUT;
    sensor.pull_up_en = GPIO_PULLUP_ENABLE;
    sensor.pull_down_en = GPIO_PULLDOWN_DISABLE;
    sensor.intr_type = GPIO_INTR_DISABLE;

    if (gpio_config(&sensor) != ESP_OK)
    {
        latchFault("sensor GPIO initialization");
        cycleRunning.store(false);
        return false;
    }

    activeDispense = {
        channel,
        profile.pulse,
        profile.durationMs
    };

    stopRequested.store(false);

    lastDispenseSlot.store(slot);
    dispenseStatus.store(DispenseStatus::Running);
    cycleStatus.store(1);

    const BaseType_t result = xTaskCreate(
        dispenseTask,
        "servo_dispense",
        4096,
        nullptr,
        5,
        nullptr
    );

    if (result != pdPASS)
    {
        latchFault("dispense task creation");
        cycleRunning.store(false);
        return false;
    }

    ESP_LOGI(
        TAG,
        "START ACCEPTED slot=%d CH%d max=%u ms",
        slot,
        channel,
        static_cast<unsigned>(profile.durationMs)
    );

    return true;
}

DispenseStatus RealDispenser::getDispenseStatus() const
{
    return dispenseStatus.load();
}

int RealDispenser::getLastDispenseSlot() const
{
    return lastDispenseSlot.load();
}

bool RealDispenser::isBusy() const
{
    return cycleRunning.load();
}

bool RealDispenser::requestStop()
{
    if (!cycleRunning.load())
    {
        return false;
    }

    stopRequested.store(true);

    return true;
}

// =====================================================
// GAME INTEGRATION
// =====================================================

// GameManager and SpecialEventManager expect:
// true  = dispensing succeeded
// false = dispensing failed.
//
// Therefore we wait for the motor task and return true
// only after a sensor-confirmed cycle.
//
// This temporarily blocks the caller (LVGL callback).
// It does not change the existing 5-second animation.

bool RealDispenser::dispense(int slot)
{
    if (!startDispense(slot))
    {
        return false;
    }

    // Bound how long the caller waits.
    // Motor task has its own per-slot deadline.

    const int channel = slot - 1;

    const int64_t deadlineUs =
        esp_timer_get_time() +
        static_cast<int64_t>(
            profiles[channel].durationMs + 1000
        ) * 1000;

    while (
        isBusy() &&
        esp_timer_get_time() < deadlineUs
    )
    {
        vTaskDelay(1);
    }

    // Worker did not complete by the extra deadline.

    if (isBusy())
    {
        ESP_LOGE(
            TAG,
            "WORKER STALLED slot=%d; requesting STOP",
            slot
        );

        requestStop();

        const int64_t stopDeadlineUs =
            esp_timer_get_time() + 250000;

        while (
            isBusy() &&
            esp_timer_get_time() < stopDeadlineUs
        )
        {
            vTaskDelay(1);
        }

        if (isBusy())
        {
            ESP_LOGE(
                TAG,
                "SERVO MAY STILL RUN: SWITCH POWER OFF"
            );
        }

        return false;
    }

    const DispenseStatus result =
        getDispenseStatus();

    const bool confirmed =
        result == DispenseStatus::CandyDetected;

    ESP_LOGI(
        TAG,
        "GAME DISPENSE slot=%d detected=%d status=%d",
        slot,
        static_cast<int>(confirmed),
        static_cast<int>(result)
    );

    return confirmed;
}