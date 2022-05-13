#include <string.h>
#include <stdint.h>
#include <driver/gpio.h>

#include <utils.h>

#include "serial-communication.h"
#include "ble_hid/ble_hidd_demo_main.h"


void echoTask() {
    char lineBuffer[SERIAL_BUFFER_SIZE]; 
    serialInit();
    uint8_t report[8] = {0};
    char str[17] = {0};

    while(true) {
        while(serialReadLine(lineBuffer, SERIAL_BUFFER_SIZE)) {
            if(lineBuffer[0] == 'R') {
                int reportLength = strlen(lineBuffer + 1)/2;
                hex_to_data(lineBuffer + 1, report, reportLength);
                hid_dev_send_report(hidd_le_env.gatt_if, hid_conn_id,
                        HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT, reportLength, report);
                
                data_to_hex(report, reportLength, str);
                serialPrintln("Received report: ");
                serialPrintln(str);
            } else {
                serialPrintln("Received line: ");
                serialPrintln(lineBuffer);
            }
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

    ble_hidd_init();

    // Create the task, storing the handle.  Note that the passed parameter ucParameterToPass
    // must exist for the lifetime of the task, so in this case is declared static.  If it was just an
    // an automatic stack variable it might no longer exist, or at least have been corrupted, by the time
    // the new task attempts to access it.
    xTaskCreate( echoTask, "echo", 1024 * 8, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle );
    configASSERT( xHandle );

    while (true) {
        vTaskDelay(1000/portTICK_RATE_MS);
    }
}