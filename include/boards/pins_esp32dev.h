#pragma once

// NodeMCU ESP32-S (classic ESP32 / WROOM). See WIRING.md.

// MAX98357A I2S
static const int PIN_I2S_DIN = 25;
static const int PIN_I2S_BCLK = 26;
static const int PIN_I2S_LRCLK = 27;

// Shared SPI (PN532 + SD), VSPI
static const int PIN_SPI_SCK = 18;
static const int PIN_SPI_MISO = 19;
static const int PIN_SPI_MOSI = 23;
static const int PIN_SD_CS = 15;
static const int PIN_PN532_SS = 5;
static const int PIN_PN532_RST = 4;

#define NFCNOISY_STATUS_LED_GPIO 1
static const int PIN_STATUS_LED = 2;

static const uint32_t SPI_SD_HZ = 4000000;

static const char *SOUNDS_DIR = "/sounds";
static const char *MAPPING_PATH = "/mapping.txt";
