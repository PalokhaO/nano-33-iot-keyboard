#ifndef __DEVICES_H_
#define __DEVICES_H_

#include <zephyr.h>
#include <device.h>

#define serialUsb DEVICE_DT_GET(DT_CHOSEN(zephyr_console))
#define serialNina DEVICE_DT_GET(DT_ALIAS(sercom_3))
#define gpioA DEVICE_DT_GET(DT_ALIAS(port_a))
#define gpioB DEVICE_DT_GET(DT_ALIAS(port_b))

#endif //__DEVICES_H_
