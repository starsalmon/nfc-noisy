#pragma once

// Unexpected Maker FeatherS3 (ESP32-S3). Feather header labels in comments.
// Avoid: GPIO2 (VBAT), GPIO4 (ambient light), GPIO8/9 (STEMMA I2C), GPIO13 (blue LED),
// GPIO34 (VBUS), GPIO39/40 (RGB LDO2 / NeoPixel), GPIO35–37 used for SPI bus.

// MAX98357A I2S — A0, A5, A6
static const int PIN_I2S_DIN = 1;   // A0
static const int PIN_I2S_BCLK = 6;  // A5
static const int PIN_I2S_LRCLK = 7; // A6

// Shared SPI — board default MOSI/SCK/MISO (Feather SPI breakout)
static const int PIN_SPI_SCK = 36;
static const int PIN_SPI_MISO = 37;
static const int PIN_SPI_MOSI = 35;
static const int PIN_SD_CS = 10;    // A9
static const int PIN_PN532_SS = 5;  // SS / A4
static const int PIN_PN532_RST = 14; // T14

#define NFCNOISY_STATUS_RGB_BUILTIN 1
// RGB_PWR (LDO2) and RGB_DATA come from variant pins_arduino.h (GPIO 39 / 40).

static const uint32_t SPI_SD_HZ = 4000000;

static const char *SOUNDS_DIR = "/sounds";
static const char *MAPPING_PATH = "/mapping.txt";
