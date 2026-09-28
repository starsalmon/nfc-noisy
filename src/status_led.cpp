#include "status_led.h"

#include "pins.h"

static void led_on(bool on) {
    // NodeMCU ESP32-S onboard LED is usually active-low on GPIO 2.
    digitalWrite(PIN_STATUS_LED, on ? LOW : HIGH);
}

void led_begin() {
    pinMode(PIN_STATUS_LED, OUTPUT);
    led_boot();
}

void led_set(uint8_t r, uint8_t g, uint8_t b) {
    led_on((r | g | b) != 0);
}

void led_boot() { led_on(true); }
void led_ready() { led_on(true); }
void led_play() { led_on(true); }
void led_setup() { led_on(true); }
void led_learn() { led_on(true); }
void led_error() {
    led_on(true);
}
void led_unknown() { led_on(true); }
