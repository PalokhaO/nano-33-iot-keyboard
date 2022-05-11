/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>

#define NINA_GPIO0 27 
#define NINA_RESETN 8
#define NINA_ACK 28 

const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
const struct device *nina = DEVICE_DT_GET(DT_ALIAS(sercom_3));
const struct device *gpioa = DEVICE_DT_GET(DT_ALIAS(port_a));

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");


struct uart_config nina_conf = {
    .baudrate=115200,
    .data_bits=UART_CFG_DATA_BITS_8,
    .flow_ctrl=UART_CFG_FLOW_CTRL_NONE,
    .parity=UART_CFG_PARITY_NONE,
    .stop_bits=UART_CFG_STOP_BITS_1
};

uint8_t usbRecvBuf[1024];
uint8_t ninaRecvBuf[1024];

void setNinaProgrammable(bool programmable) {

    gpio_pin_configure(gpioa, NINA_GPIO0, GPIO_OUTPUT);
    gpio_pin_configure(gpioa, NINA_RESETN, GPIO_OUTPUT);
    gpio_pin_configure(gpioa, NINA_ACK, GPIO_OUTPUT);
//   pinMode(NINA_GPIO0,  OUTPUT);
//   pinMode(NINA_RESETN, OUTPUT);
//   pinMode(NINA_ACK, OUTPUT);
    gpio_pin_set(gpioa, NINA_GPIO0, !programmable);
    gpio_pin_set(gpioa, NINA_ACK, false);
//   digitalWrite(NINA_GPIO0, !programmable);
//   digitalWrite(NINA_ACK, LOW);

    gpio_pin_set(gpioa, NINA_RESETN, true);
    k_sleep(K_MSEC(100));
    gpio_pin_set(gpioa, NINA_RESETN, false);
    k_sleep(K_MSEC(100));
    gpio_pin_set(gpioa, NINA_RESETN, true);
    k_sleep(K_MSEC(100));
//   digitalWrite(NINA_RESETN, HIGH);
//   delay(100);
//   digitalWrite(NINA_RESETN, LOW);
//   delay(100);
//   digitalWrite(NINA_RESETN, HIGH);
//   delay(100);

//   baud = 115200;
//   restart_serials();
}

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

    uart_config_get(dev, &nina_conf);
    if (nina_conf.baudrate != 152000) {
        gpio_pin_set(gpioa, 17, 1);
    } else {
        gpio_pin_set(gpioa, 17, 0);
    }
    // uart_configure(forwardTo, &nina_conf);

	if (uart_irq_rx_ready(dev)) {
        int read;
		while( (read = uart_fifo_read(dev, &buffer, 1)) ) {
            uart_poll_out(forwardTo, buffer);
        }
	}
}

void main(void)
{
    gpio_pin_configure(gpioa, 17, GPIO_OUTPUT_LOW);
	uint32_t dtr = 0;

	if (usb_enable(NULL)) {
		return;
	}

	/* Poll if the DTR flag was set */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}

    while(!device_is_ready(nina)) {
        printk("NINA device is not ready!");
    }
    uart_configure(nina, &nina_conf);
    uart_irq_rx_enable(nina);
    uart_irq_callback_user_data_set(nina, interrupt_handler, dev);

    uart_configure(dev, &nina_conf);
    uart_irq_rx_enable(dev);
    uart_irq_callback_user_data_set(dev, interrupt_handler, nina);

    while(!device_is_ready(gpioa)) {
        printk("gpioa device is not ready!");
    }
    
    setNinaProgrammable(true);

	while (1) {
        // if (!uart_poll_in(dev, &ch)) {
        //     uart_poll_out(nina, ch);
        // }
        // if (!uart_poll_in(nina, &ch)) {
        //     uart_poll_out(dev, ch);
        // }
		// printk("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(K_MSEC(1000));
        // printk("Heartbeat\n");
	}
}
