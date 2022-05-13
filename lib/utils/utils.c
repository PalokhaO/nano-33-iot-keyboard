#include "utils.h"

void data_to_hex(uint8_t* report, size_t reportLength, char* output) {
    for (unsigned int i = 0; i < reportLength; i++) {
        uint8_t nib1 = (report[i] >> 4) & 0x0F;
        uint8_t nib2 = (report[i] >> 0) & 0x0F;
        output[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        output[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    output[reportLength * 2] = '\0';
}

size_t hex_to_data(char* string, uint8_t* buffer, size_t bufferLength) {
    size_t copied = bufferLength < (strlen(string)/2)
        ? bufferLength
        : (strlen(string) /2);
    for(int i = 0; i < copied*2; i++) {
        uint8_t value = (string[i] > '9')
            ? (string[i] - 'A' + 10)
            : (string[i] - '0');
        buffer[i/2] = (i % 2)
            ? (buffer[i/2] + value)
            : (value << 4);
    }
    return copied;
}
