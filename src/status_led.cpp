#include "status_led.h"

#include "board_power.h"
#include "pins.h"

#if defined(NFCNOISY_STATUS_NEOPIXEL)
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel s_pixel(1, PIN_RGB_DATA, NEO_GRB + NEO_KHZ800);

static void neopixel_bus_begin() {
    pinMode(PIN_RGB_PWR, OUTPUT);
    digitalWrite(PIN_RGB_PWR, HIGH);
    s_pixel.begin();
    s_pixel.setBrightness(48);
}

#elif defined(NFCNOISY_STATUS_RGB_BUILTIN)
#include "pins_arduino.h"

static void feathers3_rgb_begin() {
    board_power_early_init();
}

#endif

#if defined(NFCNOISY_STATUS_LED_GPIO)
static void led_gpio_on(bool on) {
    digitalWrite(PIN_STATUS_LED, on ? LOW : HIGH);
}
#endif

void led_begin() {
#if defined(NFCNOISY_STATUS_NEOPIXEL)
    neopixel_bus_begin();
#elif defined(NFCNOISY_STATUS_RGB_BUILTIN)
    feathers3_rgb_begin();
#elif defined(NFCNOISY_STATUS_LED_GPIO)
    pinMode(PIN_STATUS_LED, OUTPUT);
#endif
    led_boot();
}

void led_set(uint8_t r, uint8_t g, uint8_t b) {
#if defined(NFCNOISY_STATUS_NEOPIXEL)
    s_pixel.setPixelColor(0, s_pixel.Color(r, g, b));
    s_pixel.show();
#elif defined(NFCNOISY_STATUS_RGB_BUILTIN)
    rgbLedWrite(RGB_BUILTIN, r, g, b);
#elif defined(NFCNOISY_STATUS_LED_GPIO)
    led_gpio_on((r | g | b) != 0);
#else
    (void)r;
    (void)g;
    (void)b;
#endif
}

void led_boot() { led_set(32, 32, 32); }
void led_ready() { led_set(0, 180, 0); }
void led_play() { led_set(0, 80, 200); }
void led_setup() { led_set(200, 0, 200); }
void led_learn() { led_set(200, 120, 0); }
void led_error() { led_set(220, 0, 0); }
void led_unknown() { led_set(180, 180, 0); }
