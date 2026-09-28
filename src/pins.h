#pragma once

#include <Arduino.h>

// NodeMCU ESP32-S (classic ESP32 / WROOM). Breadboard prototype.
// Source of truth for GPIO numbers. See WIRING.md.

// MAX98357A I2S DAC
static const int PIN_I2S_DIN = 25;    // ESP dout -> MAX98357 DIN
static const int PIN_I2S_BCLK = 26;
static const int PIN_I2S_LRCLK = 27;  // WS / LRC

// Shared SPI bus (PN532 + SD). VSPI.
static const int PIN_SPI_SCK = 18;
static const int PIN_SPI_MISO = 19;
static const int PIN_SPI_MOSI = 23;
static const int PIN_SD_CS = 15;
static const int PIN_PN532_SS = 5;
static const int PIN_PN532_RST = 4;  // RSTPDN, active low. Set -1 if tied to 3.3 V.

static const int PIN_STATUS_LED = 2;  // NodeMCU onboard LED (active low)

static const uint32_t SPI_SD_HZ = 4000000;

static const char *SOUNDS_DIR = "/sounds";
static const char *MAPPING_PATH = "/mapping.txt";
