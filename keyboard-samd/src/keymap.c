#include "keymap.h"

#include <string.h>
#include <drivers/gpio.h>
#include "devices.h"

char report_string[REPORT_LENGTH * 2 + 1] = {0};
uint8_t report[REPORT_LENGTH] = {0};

struct keypin hpins[KEY_COLUMNS] = {
    {
        .gpio=gpioB,
        .pin=11
    },
    {
        .gpio=gpioA,
        .pin=7
    },
    {
        .gpio=gpioA,
        .pin=5
    },
    {
        .gpio=gpioA,
        .pin=4
    },
    {
        .gpio=gpioA,
        .pin=6
    },
    {
        .gpio=gpioA,
        .pin=18
    },
    {
        .gpio=gpioA,
        .pin=20
    },
    {
        .gpio=gpioA,
        .pin=21
    },
    {
        .gpio=gpioA,
        .pin=16
    },
    {
        .gpio=gpioA,
        .pin=19
    },
    {
        .gpio=gpioA,
        .pin=17
    },
    {
        .gpio=gpioA,
        .pin=2
    },
    {
        .gpio=gpioB,
        .pin=2
    },
    {
        .gpio=gpioA,
        .pin=11
    },
};

struct keypin vpins[KEY_ROWS] = {
    {
        .gpio=gpioA,
        .pin=10
    },
    {
        .gpio=gpioB,
        .pin=8
    },
    {
        .gpio=gpioB,
        .pin=9
    },
    {
        .gpio=gpioA,
        .pin=9
    },
    {
        .gpio=gpioB,
        .pin=3
    },
};

uint16_t layers[KEY_LAYERS][KEY_ROWS][KEY_COLUMNS] = {
    {
        {KEY_ESC,        KEY_1,          KEY_2,        KEY_3,      KEY_4,      KEY_5,      KEY_6,      KEY_7,      KEY_8,      KEY_9,      KEY_0,          KEY_MINUS,      KEY_EQUAL,        KEY_BACKSPACE, },
        {KEY_TAB ,       KEY_Q,          KEY_W,        KEY_E,      KEY_R,      KEY_T,      KEY_Y,      KEY_U,      KEY_I,      KEY_O,      KEY_P,          KEY_LEFTBRACE,  KEY_RIGHTBRACE,   KEY_BACKSLASH, },
        {KEY_CAPSLOCK,   KEY_A,          KEY_S,        KEY_D,      KEY_F,      KEY_G,      KEY_H,      KEY_J,      KEY_K,      KEY_L,      KEY_SEMICOLON , KEY_APOSTROPHE, 0,                KEY_ENTER,     },
        {KEY_MOD_LSHIFT, 0,              KEY_Z,        KEY_X,      KEY_C,      KEY_V,      KEY_B,      KEY_N,      KEY_M,      KEY_COMMA,  KEY_DOT,        KEY_SLASH,      0,                KEY_MOD_RSHIFT,},
        {KEY_MOD_LCTRL,  KEY_MOD_LMETA , KEY_MOD_LALT, 0,          0,          0,          KEY_SPACE,  0,          0,          0,          KEY_MOD_RALT,   0 /*FN*/,       KEY_COMPOSE,      KEY_MOD_RCTRL, },
    },
    {
        {KEY_GRAVE,      KEY_F1,         KEY_F2,       KEY_F3,     KEY_F4,     KEY_F5,     KEY_F6,     KEY_F7,     KEY_F8,     KEY_F9,     KEY_F10,        KEY_F11,        KEY_F12,          KEY_DELETE, },
        {KEY_TRANSP,     KEY_TRANSP,     KEY_TRANSP,   KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP,     KEY_SYSRQ,      KEY_PAUSE,        KEY_TRANSP, },
        {KEY_TRANSP,     KEY_TRANSP,     KEY_TRANSP,   KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_INSERT, KEY_HOME,   KEY_PAGEUP,     KEY_PAGEDOWN,   KEY_TRANSP,       KEY_TRANSP, },
        {KEY_TRANSP,     KEY_TRANSP,     KEY_TRANSP,   KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_DELETE, KEY_END,        KEY_UP,         KEY_TRANSP,       KEY_TRANSP, },
        {KEY_TRANSP,     KEY_TRANSP,     KEY_TRANSP,   KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_TRANSP, KEY_LEFT,       KEY_TRANSP,     KEY_DOWN,         KEY_RIGHT, },
    },
};

bool keystate[KEY_ROWS][KEY_COLUMNS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
};

void keyboard_init() {
    for (int i = 0; i < KEY_ROWS; i++) {
        gpio_pin_configure(vpins[i].gpio, vpins[i].pin, GPIO_INPUT | GPIO_PULL_DOWN);
    }
    for (int j = 0; j < KEY_COLUMNS; j++) {
        gpio_pin_configure(hpins[j].gpio, hpins[j].pin, GPIO_OUTPUT_LOW);
    }
}

void keyboard_scan() {
    for (int j = 0; j < KEY_COLUMNS; j++) {
        gpio_pin_set(hpins[j].gpio, hpins[j].pin, 1);
        k_sleep(K_NSEC(1));
        for (int i = 0; i < KEY_ROWS; i++) {
            keystate[i][j] = gpio_pin_get(vpins[i].gpio, vpins[i].pin);
        }
        gpio_pin_set(hpins[j].gpio, hpins[j].pin, 0);
    }
    
    memset(report, 0, sizeof(report));
    int l = keystate[4][11]
        ? 1
        : 0;
    int char_i = 2;
    for (int i = 0; i < KEY_ROWS; i++) {
        for (int j = 0; j < KEY_COLUMNS; j++) {
            if (keystate[i][j]) {
                uint16_t code = layers[l][i][j] == KEY_TRANSP
                    ? layers[0][i][j]
                    : layers[l][i][j];
                bool isMod = code > 0x0100 && code < 0x0200;
                uint8_t keycode = (code << 8) >> 8;
                if (keycode) {
                    if (isMod) {
                        report[0] |= keycode;
                    } else if (char_i < sizeof(report)) {
                        report[char_i] = keycode;
                        char_i++;
                    }
                }
            }
        }
    }
}

