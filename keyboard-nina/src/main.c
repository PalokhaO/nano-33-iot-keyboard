#include "serial-communication.h"

char lineBuffer[SERIAL_BUFFER_SIZE];

void echoTask() {
    serialInit();

    while(true) {
        if(serialReadLine(lineBuffer, SERIAL_BUFFER_SIZE)) {
            serialPrintln("Received line: ");
            serialPrintln(lineBuffer);
            vTaskDelay(2/portTICK_RATE_MS);
        }
    }
}

void app_main() {

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    xTaskCreate( echoTask, "echo", 2048, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );

    while (true) {
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}