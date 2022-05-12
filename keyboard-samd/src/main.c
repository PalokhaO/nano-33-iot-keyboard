/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sys/reboot.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>

#include "devices.h"
#include "keymap.h"

#define NINA_GPIO0 27 
#define NINA_RESETN 8
#define NINA_ACK 28

void setNinaProgrammable(bool programmable) {
    printk("Setting nina programmable: %d", programmable);

    gpio_pin_configure(gpioA, NINA_GPIO0, GPIO_OUTPUT);
    gpio_pin_configure(gpioA, NINA_RESETN, GPIO_OUTPUT);
    gpio_pin_configure(gpioA, NINA_ACK, GPIO_OUTPUT);
    
    gpio_pin_set(gpioA, NINA_GPIO0, !programmable);
    gpio_pin_set(gpioA, NINA_ACK, false);

    gpio_pin_set(gpioA, NINA_RESETN, true);
    k_sleep(K_MSEC(100));
    gpio_pin_set(gpioA, NINA_RESETN, false);
    k_sleep(K_MSEC(100));
    gpio_pin_set(gpioA, NINA_RESETN, true);
    k_sleep(K_MSEC(100));
}

K_THREAD_STACK_DEFINE(my_stack_area, 1024);
struct k_thread uart_thread_data;
extern void uart_thread(void *, void *, void *);

void main(void)
{
	if (usb_enable(NULL)) {
		return;
	}
    keyboard_init();


    while(!device_is_ready(serialNina)) {
        printk("NINA device is not ready!");
    }

    while(!device_is_ready(gpioA)) {
        printk("gpioA device is not ready!");
    }
    
    
    printk("Before_thread");

    k_tid_t my_tid = k_thread_create(&uart_thread_data, my_stack_area,
        K_THREAD_STACK_SIZEOF(my_stack_area),
        uart_thread,
        NULL, NULL, NULL,
        5, 0, K_NO_WAIT
    );


    keyboard_scan();
    int prog_nina = keystate[4][11];
    setNinaProgrammable(prog_nina);

	while(true) {
        keyboard_scan();
        if (prog_nina) {
            prog_nina = !keystate[0][0];
            if (!prog_nina) {
                setNinaProgrammable(prog_nina);
            }
        } else {
            // printk(keyboard_report());
            // printk("\n");
        }
        k_sleep(K_MSEC(1));
	}
}

// void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf) {

//     LOG_PANIC();
//     // sys_reboot(SYS_REBOOT_COLD);
// }
