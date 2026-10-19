# NFCNoisy

Tap an NFC card, play that animal’s sound from an SD card.

- **PN532** over **SPI** (not I2C)
- **MAX98357A** I2S amp
- **microSD** on the same SPI bus as the PN532
- Sounds are **16-bit PCM WAV** (no MP3 decoder on-device)

**Main target:** Unexpected Maker **FeatherS3** (`um_feathers3_nfcnoisy`). Legacy builds: TinyC6 and classic ESP32 DevKit.

Wiring: [`WIRING.md`](WIRING.md). Per-board GPIO: [`include/boards/`](include/boards/). SD layout: [`sd/README.md`](sd/README.md).

## Flash (FeatherS3)

1. Copy Wi-Fi / OTA secrets (gitignored):

   ```bash
   cp include/secrets.example.h include/secrets.h
   # edit WIFI_SSID, WIFI_PASSWORD, OTA_PASSWORD
   ```

2. USB flash (native USB CDC):

   ```bash
   pio run -e um_feathers3_nfcnoisy -t upload
   pio device monitor
   ```

On boot you should hear the POST-style beep, see Wi-Fi status on serial, and the onboard RGB should turn **green** when NFC + SD are up. mDNS hostname: **`nfcnoisy.local`**.

### OTA upload

Use the same `OTA_PASSWORD` as in `secrets.h`. Set the upload password for PlatformIO:

```bash
export OTA_UPLOAD_PASSWORD='your-ota-password'
pio run -e um_feathers3_nfcnoisy_ota -t upload
```

The device must be on Wi-Fi first. Upload targets `nfcnoisy.local` via **ArduinoOTA**. NFC and audio pause during the update.

Partition table: **16 MB flash**, dual OTA app slots (`partitions_16MB_ota.csv`).

## Flash (TinyC6 / ESP32 DevKit)

```bash
pio run -e um_tinyc6_nfcnoisy -t upload    # TinyC6 + NeoPixel status
pio run -e esp32dev_nfcnoisy -t upload      # breadboard ESP32-WROOM
```

No `secrets.h` required for these envs (no Wi-Fi/OTA).

## Bench probes

| Env | Purpose |
|-----|---------|
| `um_feathers3_i2c_scan` | I2C scan on STEMMA bus (SDA 8 / SCL 9) |
| `um_feathers3_pn532_probe` | PN532 I2C firmware probe (module must be in I2C mode) |
| `um_tinyc6_pn532_probe` | PN532 I2C probe on TinyC6 |
| `esp32dev_pn532_probe` | PN532 I2C on GPIO 21/22 |

Main firmware uses **SPI** for the PN532; I2C probes are for bring-up only.

## How it behaves

**First tap ever** (no setup card stored yet) becomes the **setup card**. Keep that one aside.

**Play mode** (green LED): tap an animal card → play its WAV. Lift the card before tapping again.

**Setup mode** (magenta LED): tap the setup card.

1. It plays the next unbound sound in `/sounds/` (alphabetical).
2. Tap the animal card you want for that sound — it is saved to `/mapping.txt`.
3. Tap setup to skip a sound.
4. After the last sound, it returns to play mode.

Edit `/mapping.txt` on a computer if you would rather type UIDs by hand. Serial prints every UID.

## Wi-Fi (FeatherS3 only)

Connection runs in the **background** after boot. If the network is down, NFC and audio still work; serial logs Wi-Fi state every few seconds and retries every 15–30 s. When connected, serial shows IP and `nfcnoisy.local`.

## WAV format

Prefer **22050 Hz, mono, 16-bit PCM**. Stereo and 44100 Hz also work.

```bash
ffmpeg -i cat.mp3 -ar 22050 -ac 1 -sample_fmt s16 cat.wav
```
