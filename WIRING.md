# NFCNoisy wiring

Board: **Unexpected Maker TinyC6** (ESP32-C6)

**Source of truth:** `src/pins.h`

If your SPI wires do not match this table, change `pins.h` — do not guess in firmware.

---

## Quick pin map

| GPIO | Function | Notes |
|------|----------|--------|
| **5** | I2S DIN | MAX98357A **DIN** |
| **2** | I2S BCLK | MAX98357A **BCLK** |
| **3** | I2S LRCLK | MAX98357A **LRC** / WS |
| **6** | PN532 SS | Chip select (was I2C SDA) |
| **7** | PN532 RST | RSTPDN, active low (was I2C SCL) |
| **18** | SD CS | TinyC6 SS |
| **19** | SPI SCK | Shared: PN532 + SD |
| **20** | SPI MISO | Shared |
| **21** | SPI MOSI | Shared |
| **22** | RGB power | Onboard — firmware drives this |
| **23** | RGB data | Onboard NeoPixel — do not reuse |

IO0/IO1 are TinyC6 32 kHz crystal pins — do not use them for I2S. DIN is on **IO5**.

---

## Power

| Rail | Connect |
|------|---------|
| TinyC6 | USB |
| PN532 | **3.3 V** + GND (do not feed 5 V logic into the C6) |
| SD module | **3.3 V** + GND (3.3 V SPI module, not a 5 V shifter that hogs MISO) |
| MAX98357A VIN | **5 V** for more volume, or 3.3 V |
| MAX98357A GND | Common GND with TinyC6 |

Common **GND** across TinyC6, PN532, SD, and DAC.

---

## MAX98357A

| MAX98357A | TinyC6 |
|-----------|--------|
| DIN | GPIO **5** |
| BCLK | GPIO **2** |
| LRC | GPIO **3** |
| VIN | 5 V or 3.3 V |
| GND | GND |
| SD (shutdown) | **3.3 V** (must be high or the amp stays off) |
| GAIN | float (~12 dB) unless it is too loud |

Speaker between **OUT+** and **OUT−** (do not ground either speaker lead).

---

## PN532 (SPI)

Same module as before — set it to **SPI**, not I2C.

Typical Elechouse / cheap red board switches:

| SEL0 | SEL1 | Mode |
|------|------|------|
| OFF | ON | **SPI** |
| ON | OFF | I2C (old, unused) |

| PN532 | TinyC6 |
|-------|--------|
| SCK | GPIO **19** |
| MISO | GPIO **20** |
| MOSI | GPIO **21** |
| SS / NSS | GPIO **6** |
| RSTPDN / RST | GPIO **7** (or tie to 3.3 V and set `PIN_PN532_RST` to `-1`) |
| VCC | 3.3 V |
| GND | GND |

IRQ can stay unconnected (firmware polls).

---

## microSD (SPI)

PN532 and the SD card **share** SCK / MISO / MOSI. Each has its own CS.

| SD module | TinyC6 |
|-----------|--------|
| SCK | GPIO **19** |
| MISO | GPIO **20** |
| MOSI | GPIO **21** |
| CS | GPIO **18** |
| VCC | 3.3 V |
| GND | GND |

Format the card **FAT32**. Put WAVs in `/sounds/` — see `sd/README.md`.

Cheap 5 V SD adapters with onboard level shifters sometimes **never release MISO**, which breaks the PN532 on the same bus. Use a 3.3 V module (or one that actually tri-states MISO).

---

## WAV files

16-bit PCM WAV, 22050 or 44100 Hz, mono or stereo. Example:

```bash
ffmpeg -i cat.mp3 -ar 22050 -ac 1 -sample_fmt s16 cat.wav
```
