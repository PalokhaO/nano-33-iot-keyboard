#include <driver/gpio.h>
#include "serial-communication.h"

char lineBuffer[SERIAL_BUFFER_SIZE];

void echoTask() {
    serialInit();

    while(true) {
        while(serialReadLine(lineBuffer, SERIAL_BUFFER_SIZE)) {
            serialPrintln("Received line: ");
            serialPrintln(lineBuffer);
        }
        vTaskDelay(1/portTICK_RATE_MS);
    }
}

void vacate_samd_pins() {
    gpio_config_t config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_DISABLE,
        .pin_bit_mask = ((1ULL<<GPIO_NUM_13) | (1ULL<<GPIO_NUM_14) | (1ULL<<GPIO_NUM_21) | (1ULL<<GPIO_NUM_32)),
        .pull_down_en = false,
        .pull_up_en = false,
    };
    gpio_config(&config);
}

void app_main() {

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    xTaskCreate( echoTask, "echo", 4096, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );

    while (true) {
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}