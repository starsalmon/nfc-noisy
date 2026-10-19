#pragma once

// Unexpected Maker TinyC6. See WIRING.md.

// MAX98357A I2S
static const int PIN_I2S_DIN = 5;
static const int PIN_I2S_BCLK = 2;
static const int PIN_I2S_LRCLK = 3;

// Shared SPI (PN532 + SD)
static const int PIN_SPI_SCK = 19;
static const int PIN_SPI_MISO = 20;
static const int PIN_SPI_MOSI = 21;
static const int PIN_SD_CS = 18;
static const int PIN_PN532_SS = 6;
static const int PIN_PN532_RST = 7;

#define NFCNOISY_STATUS_NEOPIXEL 1
static const int PIN_RGB_PWR = 22;
static const int PIN_RGB_DATA = 23;

static const uint32_t SPI_SD_HZ = 4000000;

static const char *SOUNDS_DIR = "/sounds";
static const char *MAPPING_PATH = "/mapping.txt";
