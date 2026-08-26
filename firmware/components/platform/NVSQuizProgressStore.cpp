#include "NVSQuizProgressStore.h"

extern "C" {
#include "nvs.h"
#include "nvs_flash.h"
}

#include <stdexcept>


namespace {
    constexpr const char* NAMESPACE_NAME = "quiz_progress";

    constexpr const char* KEY_EXISTS   = "exists";
    constexpr const char* KEY_SEED     = "seed";
    constexpr const char* KEY_POSITION = "position";
    constexpr const char* KEY_ROUND    = "round";
    constexpr const char* KEY_VERSION  = "version";
}


bool NVSQuizProgressStore::exists() const
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


QuizProgress NVSQuizProgressStore::load() const
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
            "Failed to open quiz progress NVS."
        );
    }


    QuizProgress progress;


    err =
        nvs_get_u32(
            handle,
            KEY_SEED,
            &progress.seed
        );

    if (err != ESP_OK) {
        nvs_close(handle);

        throw std::runtime_error(
            "Failed to load quiz seed."
        );
    }


    uint32_t position = 0;

    err =
        nvs_get_u32(
            handle,
            KEY_POSITION,
            &position
        );

    if (err != ESP_OK) {
        nvs_close(handle);

        throw std::runtime_error(
            "Failed to load quiz position."
        );
    }

    progress.currentPosition =
        static_cast<std::size_t>(
            position
        );


    err =
        nvs_get_u32(
            handle,
            KEY_ROUND,
            &progress.round
        );

    if (err != ESP_OK) {
        nvs_close(handle);

        throw std::runtime_error(
            "Failed to load quiz round."
        );
    }


    err =
        nvs_get_u32(
            handle,
            KEY_VERSION,
            &progress.datasetVersion
        );

    if (err != ESP_OK) {
        nvs_close(handle);

        throw std::runtime_error(
            "Failed to load dataset version."
        );
    }


    nvs_close(handle);

    return progress;
}


void NVSQuizProgressStore::save(
    const QuizProgress& progress
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
            "Failed to open quiz progress NVS."
        );
    }


    nvs_set_u32(
        handle,
        KEY_SEED,
        progress.seed
    );


    nvs_set_u32(
        handle,
        KEY_POSITION,
        static_cast<uint32_t>(
            progress.currentPosition
        )
    );


    nvs_set_u32(
        handle,
        KEY_ROUND,
        progress.round
    );


    nvs_set_u32(
        handle,
        KEY_VERSION,
        progress.datasetVersion
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
            "Failed to commit quiz progress NVS."
        );
    }
}