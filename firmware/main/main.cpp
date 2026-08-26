#include "AppController.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


extern "C" void app_main(void)
{
    static AppController app;

    app.start();


    while (true) {

        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}