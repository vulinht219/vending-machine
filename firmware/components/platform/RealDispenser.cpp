#include "RealDispenser.h"

#include "BoardI2CManager.h"

#include "driver/i2c_master.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdint>


namespace
{

constexpr const char* TAG = "RealDispenser";

constexpr uint8_t PCA9685_ADDRESS = 0x40;

constexpr uint8_t REG_MODE1 = 0x00;
constexpr uint8_t REG_LED0_ON_L = 0x06;
constexpr uint8_t REG_PRESCALE = 0xFE;

constexpr uint8_t SERVO_PRESCALE = 121;

// Calibrated from our actual tests.
constexpr uint16_t SERVO_NEUTRAL = 323;

// Previously observed: slow clockwise rotation.
constexpr uint16_t SERVO_TEST_PULSE = 307;

constexpr uint32_t TEST_DURATION_MS = 1000;


// Shared state for this PCA9685 device.
i2c_master_dev_handle_t device = nullptr;

bool initialized = false;
bool testAlreadyRun = false;


// =====================================================
// WRITE ONE REGISTER
// =====================================================

bool writeRegister(
    uint8_t reg,
    uint8_t value
)
{
    if (device == nullptr)
    {
        return false;
    }

    const uint8_t data[2] = {
        reg,
        value
    };

    esp_err_t result = i2c_master_transmit(
        device,
        data,
        sizeof(data),
        100
    );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Register 0x%02X write failed: %s",
            reg,
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}


// =====================================================
// SET CH0 PWM PULSE
// =====================================================

bool setServo0Pulse(uint16_t pulse)
{
    if (device == nullptr || pulse > 4095)
    {
        return false;
    }

    // Auto-increment is enabled in MODE1.
    // Write all four CH0 registers in one transaction.
    const uint8_t data[5] = {
        REG_LED0_ON_L,
        0x00,
        0x00,
        static_cast<uint8_t>(pulse & 0xFF),
        static_cast<uint8_t>((pulse >> 8) & 0x0F)
    };

    esp_err_t result = i2c_master_transmit(
        device,
        data,
        sizeof(data),
        100
    );

    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "CH0 PWM write failed: %s",
            esp_err_to_name(result)
        );

        return false;
    }

    return true;
}

} // namespace


// =====================================================
// INITIALIZE PCA9685
// =====================================================

bool RealDispenser::initializeServo0Neutral()
{
    if (initialized)
    {
        return true;
    }

    i2c_master_bus_handle_t bus =
        BoardI2CManager::getBus();

    if (bus == nullptr)
    {
        ESP_LOGE(
            TAG,
            "I2C bus is null"
        );

        return false;
    }

    if (device == nullptr)
    {
        i2c_device_config_t config = {};

        config.dev_addr_length =
            I2C_ADDR_BIT_LEN_7;

        config.device_address =
            PCA9685_ADDRESS;

        config.scl_speed_hz =
            100000;

        esp_err_t result =
            i2c_master_bus_add_device(
                bus,
                &config,
                &device
            );

        if (result != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "PCA9685 registration failed: %s",
                esp_err_to_name(result)
            );

            device = nullptr;

            return false;
        }
    }

    // Sleep + auto-increment.
    if (!writeRegister(REG_MODE1, 0x30))
    {
        return false;
    }

    // Approximately 50 Hz.
    if (!writeRegister(REG_PRESCALE, SERVO_PRESCALE))
    {
        return false;
    }

    // Prepare neutral BEFORE enabling the oscillator.
    if (!setServo0Pulse(SERVO_NEUTRAL))
    {
        return false;
    }

    // Wake oscillator + retain auto-increment.
    if (!writeRegister(REG_MODE1, 0x20))
    {
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(2));

    initialized = true;

    ESP_LOGI(
        TAG,
        "PCA9685 initialized: CH0 neutral = %u",
        static_cast<unsigned>(SERVO_NEUTRAL)
    );

    return true;
}


// =====================================================
// TEMPORARY ONE-SHOT SERVO TEST
// =====================================================

bool RealDispenser::testServo0Once()
{
    if (!initialized || device == nullptr)
    {
        ESP_LOGE(
            TAG,
            "TEST FAIL: PCA9685 not initialized"
        );

        return false;
    }

    // Prevent a second run from repeated button presses.
    if (testAlreadyRun)
    {
        ESP_LOGW(
            TAG,
            "Servo test already executed"
        );

        return false;
    }

    testAlreadyRun = true;

    ESP_LOGI(
        TAG,
        "SERVO TEST START: CH0 pulse = %u",
        static_cast<unsigned>(SERVO_TEST_PULSE)
    );

    // Start slow rotation.
    if (!setServo0Pulse(SERVO_TEST_PULSE))
    {
        // Attempt to restore neutral even on failure.
        setServo0Pulse(SERVO_NEUTRAL);

        ESP_LOGE(
            TAG,
            "SERVO TEST FAIL: could not start"
        );

        return false;
    }

    // Run for approximately 200 milliseconds.
    vTaskDelay(
        pdMS_TO_TICKS(TEST_DURATION_MS)
    );

    // Always attempt to return to neutral.
    if (!setServo0Pulse(SERVO_NEUTRAL))
    {
        // One more attempt. Physical power switch
        // is still required if communication fails.
        setServo0Pulse(SERVO_NEUTRAL);

        ESP_LOGE(
            TAG,
            "SERVO TEST FAIL: stop command failed"
        );

        return false;
    }

    ESP_LOGI(
        TAG,
        "SERVO TEST FINISHED: CH0 returned to neutral %u",
        static_cast<unsigned>(SERVO_NEUTRAL)
    );

    return true;
}


// =====================================================
// EXISTING GAME BEHAVIOR — UNCHANGED
// =====================================================

bool RealDispenser::dispense(int slot)
{
    return slot >= 1 && slot <= 6;
}