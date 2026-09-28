#include "audio.h"

#include "pins.h"

#include <Arduino.h>
#include <ESP_I2S.h>
#include <FS.h>
#include <SD.h>
#include <math.h>
#include <string.h>

static I2SClass i2s;
static bool i2s_ok = false;

struct WavInfo {
    uint16_t channels;
    uint32_t sample_rate;
    uint16_t bits;
    uint32_t data_offset;
    uint32_t data_size;
};

bool audio_begin() {
    i2s.setPins(PIN_I2S_BCLK, PIN_I2S_LRCLK, PIN_I2S_DIN);
    // 32-bit Philips slots — MAX98357A is happier with this than 16-bit frames.
    i2s_ok = i2s.begin(I2S_MODE_STD, 22050, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    if (!i2s_ok) {
        Serial.println("I2S init failed");
        return false;
    }
    delay(30);
    Serial.printf("I2S ok  DIN=%d BCLK=%d LRCLK=%d  32-bit\n", PIN_I2S_DIN, PIN_I2S_BCLK, PIN_I2S_LRCLK);
    return true;
}

void audio_beep(int freq_hz, int duration_ms, int amp16) {
    if (!i2s_ok) {
        return;
    }
    const uint32_t rate = 22050;
    if (freq_hz < 80) {
        freq_hz = 80;
    }
    if (amp16 < 1) {
        amp16 = 1;
    }
    if (amp16 > 8000) {
        amp16 = 8000;
    }
    i2s.configureTX(rate, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);

    const int frames_total = (int)(rate * (uint32_t)duration_ms / 1000);
    const int half = rate / (freq_hz * 2);
    if (half < 1 || frames_total < 1) {
        return;
    }

    int32_t buf[128];
    int32_t sample = (int32_t)amp16 << 16;
    int count = 0;
    int written_frames = 0;
    while (written_frames < frames_total) {
        const int frames = min(64, frames_total - written_frames);
        for (int i = 0; i < frames; i++) {
            if (count > 0 && (count % half) == 0) {
                sample = -sample;
            }
            buf[i * 2] = sample;
            buf[i * 2 + 1] = sample;
            count++;
        }
        i2s.write(reinterpret_cast<const uint8_t *>(buf), (size_t)frames * 8);
        written_frames += frames;
    }

    memset(buf, 0, sizeof(buf));
    i2s.write(reinterpret_cast<const uint8_t *>(buf), sizeof(buf));
}

static void audio_sweep(int f0, int f1, int duration_ms, int amp16) {
    if (!i2s_ok) {
        return;
    }
    const uint32_t rate = 22050;
    i2s.configureTX(rate, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    const int frames_total = (int)(rate * (uint32_t)duration_ms / 1000);
    int32_t buf[128];
    float phase = 0.0f;
    int written = 0;
    while (written < frames_total) {
        const int frames = min(64, frames_total - written);
        for (int i = 0; i < frames; i++) {
            const float t = (float)(written + i) / (float)frames_total;
            const float freq = (float)f0 + ((float)f1 - (float)f0) * t;
            phase += 2.0f * 3.1415926f * freq / (float)rate;
            if (phase > 2.0f * 3.1415926f) {
                phase -= 2.0f * 3.1415926f;
            }
            const int32_t s = (int32_t)((sinf(phase) * (float)amp16)) << 16;
            buf[i * 2] = s;
            buf[i * 2 + 1] = s;
        }
        i2s.write(reinterpret_cast<const uint8_t *>(buf), (size_t)frames * 8);
        written += frames;
    }
    memset(buf, 0, sizeof(buf));
    i2s.write(reinterpret_cast<const uint8_t *>(buf), sizeof(buf));
}

void audio_boot_post() {
    // RAM count, HDD spin-up, then the BIOS beep (last, like a real POST).
    for (int i = 0; i < 9; i++) {
        audio_beep(2400, 5, 700);
        delay(22 - i);
    }
    delay(50);
    audio_sweep(70, 380, 260, 900);
    delay(20);
    audio_beep(1700, 8, 800);
    delay(35);
    audio_beep(1200, 6, 700);
    delay(80);
    audio_beep(1000, 90, 1400);
}

static bool parse_wav(File &f, WavInfo *info) {
    char hdr[12];
    if (f.read(reinterpret_cast<uint8_t *>(hdr), 12) != 12) {
        return false;
    }
    if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) {
        return false;
    }

    bool got_fmt = false;
    bool got_data = false;
    while (f.available()) {
        char id[4];
        uint32_t size = 0;
        if (f.read(reinterpret_cast<uint8_t *>(id), 4) != 4) {
            break;
        }
        if (f.read(reinterpret_cast<uint8_t *>(&size), 4) != 4) {
            break;
        }
        const uint32_t chunk_start = f.position();

        if (memcmp(id, "fmt ", 4) == 0) {
            uint16_t format = 0;
            uint16_t channels = 0;
            uint32_t rate = 0;
            uint16_t bits = 0;
            if (f.read(reinterpret_cast<uint8_t *>(&format), 2) != 2) {
                return false;
            }
            if (f.read(reinterpret_cast<uint8_t *>(&channels), 2) != 2) {
                return false;
            }
            if (f.read(reinterpret_cast<uint8_t *>(&rate), 4) != 4) {
                return false;
            }
            f.seek(chunk_start + 14);
            if (f.read(reinterpret_cast<uint8_t *>(&bits), 2) != 2) {
                return false;
            }
            if (format != 1) {
                Serial.printf("WAV not PCM (format=%u)\n", format);
                return false;
            }
            info->channels = channels;
            info->sample_rate = rate;
            info->bits = bits;
            got_fmt = true;
        } else if (memcmp(id, "data", 4) == 0) {
            info->data_offset = chunk_start;
            info->data_size = size;
            got_data = true;
        }

        f.seek(chunk_start + size + (size & 1));
        if (got_fmt && got_data) {
            break;
        }
    }
    return got_fmt && got_data;
}

static void write_all(const uint8_t *data, size_t len) {
    size_t off = 0;
    while (off < len) {
        const size_t n = i2s.write(data + off, len - off);
        if (n == 0) {
            delay(1);
            continue;
        }
        off += n;
    }
}

bool audio_play_wav(const char *path) {
    if (!i2s_ok || path == nullptr || path[0] == '\0') {
        return false;
    }

    File f = SD.open(path, FILE_READ);
    if (!f) {
        Serial.printf("WAV missing: %s\n", path);
        audio_beep(220, 180);
        return false;
    }

    WavInfo info{};
    if (!parse_wav(f, &info)) {
        Serial.printf("WAV header bad: %s\n", path);
        f.close();
        audio_beep(220, 180);
        return false;
    }

    if ((info.bits != 8 && info.bits != 16) || info.channels < 1 || info.channels > 2) {
        Serial.printf("WAV unsupported: %u-bit %u ch %lu Hz\n", info.bits, info.channels,
                      (unsigned long)info.sample_rate);
        f.close();
        audio_beep(220, 180);
        return false;
    }

    Serial.printf("Play %s  %lu Hz  %u-bit  %u ch\n", path, (unsigned long)info.sample_rate, info.bits,
                  info.channels);

    i2s.configureTX(info.sample_rate, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO);
    f.seek(info.data_offset);

    uint8_t in[256];
    int32_t out[256];  // 128 stereo frames
    uint32_t left = info.data_size;
    while (left > 0) {
        size_t max_in = sizeof(in);
        if (info.bits == 16 && info.channels == 1) {
            max_in = 256;
        } else if (info.bits == 8) {
            max_in = 128;
        }
        const size_t want = (size_t)min((uint32_t)max_in, left);
        const int n = f.read(in, want);
        if (n <= 0) {
            break;
        }
        left -= (uint32_t)n;

        size_t frames = 0;
        if (info.bits == 16 && info.channels == 2) {
            frames = (size_t)n / 4;
            const int16_t *src = reinterpret_cast<const int16_t *>(in);
            for (size_t i = 0; i < frames; i++) {
                out[i * 2] = ((int32_t)src[i * 2]) << 16;
                out[i * 2 + 1] = ((int32_t)src[i * 2 + 1]) << 16;
            }
        } else if (info.bits == 16 && info.channels == 1) {
            frames = (size_t)n / 2;
            const int16_t *src = reinterpret_cast<const int16_t *>(in);
            for (size_t i = 0; i < frames; i++) {
                const int32_t s = ((int32_t)src[i]) << 16;
                out[i * 2] = s;
                out[i * 2 + 1] = s;
            }
        } else if (info.bits == 8 && info.channels == 1) {
            frames = (size_t)n;
            for (size_t i = 0; i < frames; i++) {
                const int32_t s = ((int32_t)((int)in[i] - 128)) << 24;
                out[i * 2] = s;
                out[i * 2 + 1] = s;
            }
        } else {
            frames = (size_t)n / 2;
            for (size_t i = 0; i < frames; i++) {
                out[i * 2] = ((int32_t)((int)in[i * 2] - 128)) << 24;
                out[i * 2 + 1] = ((int32_t)((int)in[i * 2 + 1] - 128)) << 24;
            }
        }
        write_all(reinterpret_cast<const uint8_t *>(out), frames * 8);
    }

    memset(out, 0, sizeof(out));
    write_all(reinterpret_cast<const uint8_t *>(out), sizeof(out));
    f.close();
    return true;
}
