#ifndef __UTILS_H_
#define __UTILS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void data_to_hex(uint8_t* report, size_t reportLength, char* output);
size_t hex_to_data(char* string, uint8_t* buffer, size_t bufferLength);

#endif //__UTILS_H_