#include <zephyr.h>
#include <drivers/uart.h>
#include "devices.h"

struct uart_config nina_conf = {
    .baudrate=115200,
    .data_bits=UART_CFG_DATA_BITS_8,
    .flow_ctrl=UART_CFG_FLOW_CTRL_NONE,
    .parity=UART_CFG_PARITY_NONE,
    .stop_bits=UART_CFG_STOP_BITS_1
};

static void interrupt_handler(struct device *dev, void* data)
{
    struct device* forwardTo = (struct device*)data;
    uint8_t buffer;
    uint32_t baudrate;

	uart_irq_update(dev);

	if(
        !uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate) &&
        baudrate != nina_conf.baudrate
    ) {
        nina_conf.baudrate = baudrate;
        uart_configure(forwardTo, &nina_conf);
    }

	if (uart_irq_rx_ready(dev)) {
        int read;
		while( (read = uart_fifo_read(dev, &buffer, 1)) ) {
            uart_poll_out(forwardTo, buffer);
        }
	}
}

void uart_thread(void *a, void *b, void *c) {
    uart_configure(serialNina, &nina_conf);
    uart_irq_rx_enable(serialNina);
    uart_irq_callback_user_data_set(serialNina, interrupt_handler, serialUsb);

    uart_configure(serialUsb, &nina_conf);
    uart_irq_rx_enable(serialUsb);
    uart_irq_callback_user_data_set(serialUsb, interrupt_handler, serialNina);
    while(true) {
        k_sleep(K_NSEC(10));
    }
}