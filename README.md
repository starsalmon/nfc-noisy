# NFCNoisy

TinyC6 toy: tap an NFC card, play that animal’s sound from an SD card.

- **PN532** over **SPI** (not I2C)
- **MAX98357A** I2S amp on IO 5 / 2 / 3 (DIN / BCLK / LRCLK)
- **microSD** on the same SPI bus as the PN532
- Sounds are **16-bit PCM WAV** (simpler and more reliable than MP3 on a C6)

Wiring: [`WIRING.md`](WIRING.md). Pins: [`src/pins.h`](src/pins.h). SD card layout: [`sd/README.md`](sd/README.md).

## Flash

```bash
cd NFCNoisy
pio run -e um_tinyc6_nfcnoisy -t upload
pio device monitor
```

On boot you should hear a short beep (DAC alive) and the onboard RGB should go green once NFC + SD are up.

## How it behaves

**First tap ever** (no setup card stored yet) becomes the **setup card**. Keep that one aside.

**Play mode** (green LED): tap an animal card → play its WAV. Lift the card before tapping again.

**Setup mode** (magenta LED): tap the setup card.

1. It plays the next unbound sound in `/sounds/` (alphabetical).
2. Tap the animal card you want for that sound — it is saved to `/mapping.txt`.
3. Tap setup to skip a sound.
4. After the last sound, it returns to play mode.

Edit `/mapping.txt` on a computer if you would rather type UIDs by hand. Serial prints every UID, so you can copy them.

## WAV format

Prefer **22050 Hz, mono, 16-bit PCM**. Stereo and 44100 Hz also work. MP3 is not implemented yet.
