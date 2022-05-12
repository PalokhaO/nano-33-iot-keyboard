/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <sys/reboot.h>
#include <sys/time_units.h>
#include <logging/log.h>
#include <logging/log_ctrl.h>
#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>

#include "devices.h"
#include "keymap.h"

#define NINA_GPIO0 27 
#define NINA_RESETN 8
#define NINA_ACK 28

const struct device *hid_dev;
static const uint8_t hid_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

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
static void in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);
}

enum usb_dc_status_code usb_status;
static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
    usb_status = status;
}

static const struct hid_ops ops = {
	.int_in_ready = in_ready_cb,
};
void callback(const struct device *dev,
				struct uart_event *evt, void *user_data) {}

void main(void)
{
	if (usb_enable(status_cb)) {
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

    hid_dev = device_get_binding("HID_0");
    if (hid_dev == NULL) {
		printk("Cannot get USB HID 0 Device");
		return;
	}
    
	usb_hid_register_device(hid_dev, hid_kbd_report_desc,
        sizeof(hid_kbd_report_desc), &ops);

	if(usb_hid_init(hid_dev)) {
        printk("Cannot init USB HID");
        return;
    }
    keyboard_scan();
    int prog_nina = keystate[4][11];
    setNinaProgrammable(prog_nina);
    k_sleep(K_MSEC(500));
	while(true) {
        keyboard_scan();
        if (prog_nina) {
            prog_nina = !keystate[0][0];
            if (!prog_nina) {
                setNinaProgrammable(prog_nina);
            }
        } else {
            char* report_string = keyboard_report_string();
            if (usb_status == USB_DC_CONFIGURED) {
                if (report_string[4] != '0' || report_string[5] != '0') {
                    printk(report_string);
                }
                hid_int_ep_write(hid_dev, report, sizeof(report), NULL);
            } else {
                // printk("Usb status: %d", usb_status);

                uart_poll_out(serialNina, 'R');
                for (int i = 0; i < strlen(report_string); i++) {
                    uart_poll_out(serialNina, report_string[i]);
                }
                uart_poll_out(serialNina, '\n');
            }
        }
        k_sleep(K_MSEC(10));
	}
}