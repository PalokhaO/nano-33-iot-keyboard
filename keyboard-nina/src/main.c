#include "serial-communication.h"

char lineBuffer[SERIAL_BUFFER_SIZE];

void app_main() {
    serialInit();

    while(true) {
        if(serialReadLine(lineBuffer, SERIAL_BUFFER_SIZE)) {
            serialPrintln("Received line: ");
            serialPrintln(lineBuffer);
        }
    }
}