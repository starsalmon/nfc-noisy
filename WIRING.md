# NFCNoisy wiring

Per-board GPIO sources: `include/boards/pins_*.h`.

---

## FeatherS3 (default build)

Board: **Unexpected Maker FeatherS3** (ESP32-S3, 16 MB flash, 8 MB PSRAM)

### Quick pin map

| Feather label | GPIO | Function |
|---------------|------|----------|
| **A0** | 1 | I2S **DIN** → MAX98357A |
| **A5** | 6 | I2S **BCLK** |
| **A6** | 7 | I2S **LRCLK** (LRC / WS) |
| **SS** (A4) | 5 | PN532 **NSS** |
| **A9** | 10 | microSD **CS** |
| **T14** | 14 | PN532 **RSTPDN** (or tie RST to 3.3 V and set `PIN_PN532_RST` to `-1` in `pins_feathers3.h`) |
| **MOSI** | 35 | SPI MOSI (PN532 + SD) |
| **SCK** | 36 | SPI SCK |
| **MISO** | 37 | SPI MISO |
| **LDO2** / RGB_PWR | 39 | Enables **LDO2 3.3 V** (firmware holds HIGH) |
| *(onboard)* | 40 | NeoPixel data |

**Peripheral power:** Wire **MAX98357A VIN**, **PN532 VCC**, and **SD VCC** to the Feather **LDO2 3.3 V** output (not the main 3V3 pin if you share the owner’s harness). Firmware turns **GPIO 39 (RGB_PWR)** on at boot and leaves it on; dimming the status LED is done via the NeoPixel only.

**Do not use for peripherals:** GPIO **2** (VBAT sense), **4** (ambient light), **8** / **9** (STEMMA I2C), **13** (blue LED), **34** (VBUS), **39** / **40** (RGB), **43** / **44** (USB-serial UART).

### MAX98357A

| MAX98357A | FeatherS3 |
|-----------|-----------|
| DIN | **A0** (GPIO 1) |
| BCLK | **A5** (GPIO 6) |
| LRC | **A6** (GPIO 7) |
| VIN | **LDO2 3.3 V** (this build) or 5 V / main 3V3 if wired separately |
| GND | GND |
| SD (shutdown) | **3.3 V** (high = amp on) |
| GAIN | float (~12 dB) unless too loud |

### PN532 (SPI)

Set module switches to **SPI** (SEL0 OFF, SEL1 ON on typical red boards).

| PN532 | FeatherS3 |
|-------|-----------|
| SCK | **SCK** (GPIO 36) |
| MISO | **MISO** (GPIO 37) |
| MOSI | **MOSI** (GPIO 35) |
| SS | **SS** (GPIO 5) |
| RSTPDN | **T14** (GPIO 14) or 3.3 V |
| VCC | 3.3 V |
| GND | GND |

### microSD (SPI)

Shared SCK / MISO / MOSI with PN532; separate chip select.

| SD module | FeatherS3 |
|-----------|-----------|
| SCK | GPIO **36** |
| MISO | GPIO **37** |
| MOSI | GPIO **35** |
| CS | **A9** (GPIO 10) |
| VCC | 3.3 V |
| GND | GND |

Use a **3.3 V** SPI module (avoid 5 V level shifters that lock MISO).

### Power

| Rail | Connect |
|------|---------|
| FeatherS3 | USB-C |
| PN532 | 3.3 V + GND |
| SD | 3.3 V + GND |
| MAX98357A | 5 V or 3.3 V + common GND |

---

## TinyC6

Board: **Unexpected Maker TinyC6** (ESP32-C6)

| GPIO | Function |
|------|----------|
| **5** | I2S DIN |
| **2** | I2S BCLK |
| **3** | I2S LRCLK |
| **6** | PN532 SS |
| **7** | PN532 RST |
| **18** | SD CS |
| **19** | SPI SCK |
| **20** | SPI MISO |
| **21** | SPI MOSI |
| **22** / **23** | Onboard RGB power / data |

IO **0** / **1** are the 32 kHz crystal — do not use for I2S.

---

## Classic ESP32 DevKit (esp32dev)

| GPIO | Function |
|------|----------|
| **25** | I2S DIN |
| **26** | I2S BCLK |
| **27** | I2S LRCLK |
| **18** | SPI SCK |
| **19** | SPI MISO |
| **23** | SPI MOSI |
| **15** | SD CS |
| **5** | PN532 SS |
| **4** | PN532 RST |
| **2** | Onboard LED (active low) |

---

## WAV files

16-bit PCM WAV, 22050 or 44100 Hz, mono or stereo:

```bash
ffmpeg -i cat.mp3 -ar 22050 -ac 1 -sample_fmt s16 cat.wav
```

Format SD card **FAT32**. Put WAVs in `/sounds/` — see `sd/README.md`.
