#pragma once

#include <Arduino.h>

void wifi_ota_begin();
void wifi_ota_loop();

// True while an OTA flash is in progress — pause NFC polling and audio playback.
bool wifi_ota_active();
