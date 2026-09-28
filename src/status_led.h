#pragma once

#include <Arduino.h>

void led_begin();
void led_set(uint8_t r, uint8_t g, uint8_t b);

void led_boot();
void led_ready();
void led_play();
void led_setup();
void led_learn();
void led_error();
void led_unknown();
