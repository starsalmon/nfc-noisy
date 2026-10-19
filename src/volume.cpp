#include "volume.h"

#include "audio.h"

#include <Arduino.h>
#include <Preferences.h>

static Preferences s_prefs;
static int s_percent = 80;
static bool s_loaded = false;

// Full-scale digital gain cap (100% slider ≈ this linear gain).
static const float kMaxGain = 0.82f;

static void volume_save() {
    s_prefs.putUChar("pct", (uint8_t)s_percent);
}

static float volume_gain() {
    if (s_percent <= 0) {
        return 0.0f;
    }
    return (float)s_percent / 100.0f * kMaxGain;
}

void volume_begin() {
    if (s_loaded) {
        return;
    }
    s_prefs.begin("nfcnoisy", false);
    s_percent = (int)s_prefs.getUChar("pct", 80);
    if (s_percent < 0) {
        s_percent = 0;
    }
    if (s_percent > 100) {
        s_percent = 100;
    }
    s_loaded = true;
    Serial.printf("Volume %d%%\n", s_percent);
}

int volume_get_percent() { return s_percent; }

bool volume_set_percent(int percent) {
    if (percent < 0) {
        percent = 0;
    }
    if (percent > 100) {
        percent = 100;
    }
    s_percent = percent;
    volume_save();
    return true;
}

bool volume_step_up() {
    const int next = s_percent + 5;
    return volume_set_percent(next > 100 ? 100 : next);
}

bool volume_step_down() {
    const int next = s_percent - 5;
    return volume_set_percent(next < 0 ? 0 : next);
}

int32_t volume_scale_i32(int32_t sample) {
    const float g = volume_gain();
    const float scaled = (float)sample * g;
    if (scaled > 2147483647.0f) {
        return INT32_MAX;
    }
    if (scaled < -2147483648.0f) {
        return INT32_MIN;
    }
    return (int32_t)scaled;
}

int volume_scale_amp(int amp16) {
    if (s_percent <= 0) {
        return 0;
    }
    const int scaled = (int)((float)amp16 * volume_gain());
    if (scaled < 1) {
        return 1;
    }
    if (scaled > 8000) {
        return 8000;
    }
    return scaled;
}

void volume_play_feedback() {
    if (s_percent <= 0) {
        audio_beep(220, 120, volume_scale_amp(1400));
        return;
    }
    if (s_percent >= 100) {
        audio_beep(1100, 50, volume_scale_amp(1200));
        audio_beep(1100, 50, volume_scale_amp(1200));
        return;
    }
    const int freq = 320 + s_percent * 7;
    audio_beep(freq, 70, volume_scale_amp(1600));
}
