#ifndef __SERIAL_COMMUNICATION__
#define __SERIAL_COMMUNICATION__

#include "driver/uart.h"

#define SERIAL_NUM UART_NUM_0
#define SERIAL_BUFFER_SIZE 1024

void serialInit();

void serialPrintln(char* text);

int serialReadLine(char* buffer, size_t maxLength);

#endif //__SERIAL_COMMUNICATION__
