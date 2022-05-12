#include "serial-communication.h"
#include "driver/gpio.h"
#include <string.h>

QueueHandle_t SerialEventQueue;
QueueHandle_t SerialDataQueue;
char lineBuffer[SERIAL_BUFFER_SIZE];
int lineBufferPos = 0;

void serialInit() {
    // Vacate pins connected to the Nano's external pins
    gpio_config_t config;
    PIN_FUNC_SELECT( IO_MUX_GPIO13_REG, PIN_FUNC_GPIO);
    PIN_FUNC_SELECT( IO_MUX_GPIO14_REG, PIN_FUNC_GPIO);
    config.pin_bit_mask = (1ULL<<GPIO_NUM_21) | (1ULL<<GPIO_NUM_13) | (1ULL<<GPIO_NUM_14);
	config.mode = GPIO_MODE_INPUT;
	config.pull_up_en = GPIO_PULLUP_DISABLE;
	config.pull_down_en = GPIO_PULLDOWN_DISABLE; 
	config.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&config));

    // Setup the actual UART    
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_driver_install(SERIAL_NUM, SERIAL_BUFFER_SIZE * 2, 0, SERIAL_BUFFER_SIZE, &SerialEventQueue, 0));
    ESP_ERROR_CHECK(uart_param_config(SERIAL_NUM, &uart_config));
    assert(SerialDataQueue = xQueueCreate( SERIAL_BUFFER_SIZE, sizeof( char ) ));

}

void drainEventQueue() {
    uart_event_t event;
    assert(SerialDataQueue);
    char* dataPacketBuffer;

    while(xQueueReceive( SerialEventQueue, &(event), 0 )) {
        switch(event.type) {
            case UART_DATA:
                dataPacketBuffer = malloc(event.size);
                int read = uart_read_bytes(SERIAL_NUM, dataPacketBuffer, event.size, 0);
                for (int i = 0; i < read; i++) {
                    xQueueSend(SerialDataQueue, dataPacketBuffer + i, 0);
                }
                free(dataPacketBuffer);
                break;
            default:
                break;
        }
    }
}

void serialPrintln(char* text) {
    uart_write_bytes(SERIAL_NUM, text, strlen(text));
    uart_write_bytes(SERIAL_NUM, "\n", sizeof(char));
}

int serialReadLine(char* buffer, size_t maxLength) {
    drainEventQueue();

    while(xQueueReceive(SerialDataQueue, lineBuffer + lineBufferPos, 0)) {
        if(lineBuffer[lineBufferPos] == '\n') {
            int charLength = lineBufferPos < (maxLength-1)
                ? lineBufferPos + 1
                : (maxLength - 1);
            strncpy(buffer, lineBuffer, charLength);
            lineBuffer[charLength] = '\0';
            lineBufferPos = 0;
            return charLength;
        } else {
            lineBufferPos++;
        }
    }
    return 0;
}
