#pragma once

#include <Arduino.h>

#if defined(NFCNOISY_BOARD_FEATHERS3)

#include "pins_arduino.h"

inline void board_power_early_init() {
    static bool s_ldo2_enabled = false;
    if (s_ldo2_enabled) {
        return;
    }
    pinMode(RGB_PWR, OUTPUT);
    digitalWrite(RGB_PWR, HIGH);
    delay(80);
    s_ldo2_enabled = true;
}

#else

inline void board_power_early_init() {}

#endif
