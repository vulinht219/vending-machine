#include "NVSGameProgressStore.h"

extern "C" {
#include "nvs.h"
}

#include <stdexcept>


namespace {
    constexpr const char* NAMESPACE_NAME = "game_progress";

    constexpr const char* KEY_EXISTS =
        "exists";

    constexpr const char* KEY_PENDING_REWARD =
        "pending";
}


bool NVSGameProgressStore::exists() const
{
    nvs_handle_t handle;

    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READONLY,
            &handle
        );

    if (err != ESP_OK) {
        return false;
    }


    uint8_t existsFlag = 0;


    err =
        nvs_get_u8(
            handle,
            KEY_EXISTS,
            &existsFlag
        );


    nvs_close(handle);


    return
        err == ESP_OK &&
        existsFlag == 1;
}


GameProgress NVSGameProgressStore::load() const
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READONLY,
            &handle
        );


    if (err != ESP_OK) {
        throw std::runtime_error(
            "Failed to open game progress NVS."
        );
    }


    uint8_t pendingReward = 0;


    err =
        nvs_get_u8(
            handle,
            KEY_PENDING_REWARD,
            &pendingReward
        );


    nvs_close(handle);


    if (err != ESP_OK) {
        throw std::runtime_error(
            "Failed to load pending reward."
        );
    }


    GameProgress progress;

    progress.pendingReward =
        pendingReward != 0;


    return progress;
}


void NVSGameProgressStore::save(
    const GameProgress& progress
) const
{
    nvs_handle_t handle;


    esp_err_t err =
        nvs_open(
            NAMESPACE_NAME,
            NVS_READWRITE,
            &handle
        );


    if (err != ESP_OK) {
        throw std::runtime_error(
            "Failed to open game progress NVS."
        );
    }


    nvs_set_u8(
        handle,
        KEY_PENDING_REWARD,
        progress.pendingReward
            ? 1
            : 0
    );


    nvs_set_u8(
        handle,
        KEY_EXISTS,
        1
    );


    err =
        nvs_commit(handle);


    nvs_close(handle);


    if (err != ESP_OK) {
        throw std::runtime_error(
            "Failed to commit game progress NVS."
        );
    }
}