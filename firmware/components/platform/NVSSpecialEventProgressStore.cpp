#include "NVSSpecialEventProgressStore.h"

extern "C" {
#include "nvs.h"
}

#include <stdexcept>


namespace {

constexpr const char* NAMESPACE_NAME =
    "special_event";

constexpr const char* KEY_EXISTS =
    "exists";

constexpr const char* KEY_YEAR =
    "year";

constexpr const char* KEY_MONTH =
    "month";

constexpr const char* KEY_DAY =
    "day";

constexpr const char* KEY_STATE =
    "state";

constexpr const char* KEY_SLOT =
    "slot";

}


bool NVSSpecialEventProgressStore::
exists() const
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READONLY,
            &handle
        );


    if (
        err != ESP_OK
    ) {
        return false;
    }


    uint8_t existsFlag = 0;


    err =
        nvs_get_u8(
            handle,
            KEY_EXISTS,
            &existsFlag
        );


    nvs_close(
        handle
    );


    return
        err == ESP_OK &&
        existsFlag == 1;
}


SpecialEventProgress
NVSSpecialEventProgressStore::
load() const
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READONLY,
            &handle
        );


    if (
        err != ESP_OK
    ) {
        throw std::runtime_error(
            "Failed to open special event NVS."
        );
    }


    SpecialEventProgress progress;


    int32_t year = 0;
    int32_t month = 0;
    int32_t day = 0;
    int32_t state = 0;
    int32_t slot = 0;


    nvs_get_i32(
        handle,
        KEY_YEAR,
        &year
    );

    nvs_get_i32(
        handle,
        KEY_MONTH,
        &month
    );

    nvs_get_i32(
        handle,
        KEY_DAY,
        &day
    );

    nvs_get_i32(
        handle,
        KEY_STATE,
        &state
    );

    nvs_get_i32(
        handle,
        KEY_SLOT,
        &slot
    );


    nvs_close(
        handle
    );


    progress.date = {
        year,
        month,
        day
    };


    progress.state =
        static_cast<
            SpecialRewardState
        >(
            state
        );


    progress.selectedSlot =
        slot;


    return progress;
}


void NVSSpecialEventProgressStore::
save(
    const SpecialEventProgress& progress
) const
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READWRITE,
            &handle
        );


    if (
        err != ESP_OK
    ) {
        throw std::runtime_error(
            "Failed to open special event NVS."
        );
    }


    nvs_set_i32(
        handle,
        KEY_YEAR,
        progress.date.year
    );

    nvs_set_i32(
        handle,
        KEY_MONTH,
        progress.date.month
    );

    nvs_set_i32(
        handle,
        KEY_DAY,
        progress.date.day
    );

    nvs_set_i32(
        handle,
        KEY_STATE,
        static_cast<int32_t>(
            progress.state
        )
    );

    nvs_set_i32(
        handle,
        KEY_SLOT,
        progress.selectedSlot
    );

    nvs_set_u8(
        handle,
        KEY_EXISTS,
        1
    );


    err =
        nvs_commit(
            handle
        );


    nvs_close(
        handle
    );


    if (
        err != ESP_OK
    ) {
        throw std::runtime_error(
            "Failed to commit special event NVS."
        );
    }
}