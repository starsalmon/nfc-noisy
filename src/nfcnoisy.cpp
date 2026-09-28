#include "audio.h"
#include "mapping.h"
#include "nfc.h"
#include "pins.h"
#include "status_led.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

enum class Mode { Play, Setup, LearnSetup };

static Mode mode = Mode::Play;
static int setup_index = 0;
static char last_uid[kUidLen];
static bool card_held = false;
static bool sd_ok = false;
static bool nfc_ok = false;

static void restore_idle_led() {
    if (mode == Mode::Setup) {
        led_setup();
    } else if (mode == Mode::LearnSetup) {
        led_learn();
    } else {
        led_ready();
    }
}

static void spi_bus_begin() {
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SD_CS);
}

static bool sd_begin() {
    for (int attempt = 0; attempt < 3; attempt++) {
        if (SD.begin(PIN_SD_CS, SPI, SPI_SD_HZ)) {
            SD.mkdir(SOUNDS_DIR);
            Serial.println("SD ok");
            return true;
        }
        Serial.printf("SD begin failed (%d)\n", attempt + 1);
        SD.end();
        delay(200);
    }
    return false;
}

static void preview_current() {
    const char *path = mapping_sound_path(setup_index);
    if (!path) {
        return;
    }
    Serial.printf("Setup preview [%d/%d] %s  (hearing the file — this tap is not bound yet unless you just assigned)\n", setup_index + 1, mapping_sound_count(), path);
    led_setup();
    audio_play_wav(path);
    led_setup();
}

static void enter_setup() {
    mapping_scan_sounds();
    if (mapping_sound_count() == 0) {
        Serial.println("Setup: no wav files in /sounds");
        led_error();
        audio_beep(200, 200);
        delay(200);
        mode = Mode::Play;
        restore_idle_led();
        return;
    }
    mode = Mode::Setup;
    setup_index = 0;
    led_setup();
    audio_beep(660, 70);
    audio_beep(880, 70);
    preview_current();
}

static void finish_setup() {
    Serial.println("Setup done");
    mode = Mode::Play;
    audio_beep(880, 60);
    audio_beep(1175, 80);
    led_ready();
}

static void handle_card(const char *uid) {
    Serial.printf("Card %s\n", uid);

    if (mode == Mode::LearnSetup) {
        mapping_set_setup_uid(uid);
        if (sd_ok) {
            mapping_save();
        }
        Serial.printf("Setup card learned: %s\n", uid);
        Serial.println("That tap is the SETUP card, not an animal. Next you'll hear previews.");
        audio_beep(880, 80);
        audio_beep(1320, 120);
        enter_setup();
        return;
    }

    if (mapping_is_setup(uid)) {
        if (mode == Mode::Play) {
            enter_setup();
            return;
        }
        setup_index++;
        if (setup_index >= mapping_sound_count()) {
            finish_setup();
        } else {
            preview_current();
        }
        return;
    }

    if (mode == Mode::Setup) {
        const char *path = mapping_sound_path(setup_index);
        if (!path) {
            finish_setup();
            return;
        }
        mapping_bind(uid, path);
        if (sd_ok) {
            mapping_save();
        }
        Serial.printf("Bound %s -> %s\n", uid, path);
        audio_beep(1200, 50);
        setup_index++;
        if (setup_index >= mapping_sound_count()) {
            finish_setup();
        } else {
            preview_current();
        }
        return;
    }

    const char *path = mapping_path_for_uid(uid);
    if (!path) {
        Serial.println("Unmapped card (tap setup card to assign)");
        led_unknown();
        audio_beep(240, 120);
        restore_idle_led();
        return;
    }
    led_play();
    audio_play_wav(path);
    restore_idle_led();
}

void setup() {
    Serial.begin(115200);
    delay(1200);
    Serial.println("\nNFCNoisy  NodeMCU ESP32-S  PN532 SPI + MAX98357A + SD");

    led_begin();

    if (audio_begin()) {
        audio_boot_post();
    } else {
        led_error();
    }

    spi_bus_begin();
    sd_ok = sd_begin();
    if (sd_ok) {
        mapping_load();
        const char *boot_wav = nullptr;
        if (SD.exists("/sounds/67.wav")) {
            boot_wav = "/sounds/67.wav";
        } else if (mapping_sound_count() > 0) {
            boot_wav = mapping_sound_path(0);
        }
        if (boot_wav) {
            Serial.printf("Boot play %s\n", boot_wav);
            led_play();
            if (!audio_play_wav(boot_wav)) {
                Serial.println("Boot WAV play failed");
            }
            restore_idle_led();
        } else {
            Serial.println("No WAV found to play (see SD listing above)");
        }
    } else {
        led_error();
        Serial.println("SD missing — NFC still works, playback needs a card");
    }

    nfc_ok = nfc_begin();
    if (!nfc_ok) {
        led_error();
    }

    last_uid[0] = '\0';
    if (!nfc_ok) {
        Serial.println("Halted: no PN532. Fix SPI / SEL switches.");
        return;
    }

    if (mapping_setup_uid()[0] == '\0') {
        mode = Mode::LearnSetup;
        led_learn();
        Serial.println("Tap a card to make it the SETUP card");
        audio_beep(520, 80);
    } else {
        mode = Mode::Play;
        led_ready();
        Serial.printf("Ready. Setup card %s\n", mapping_setup_uid());
        mapping_dump();
    }
}

void loop() {
    if (!nfc_ok) {
        delay(500);
        return;
    }

    char uid[kUidLen];
    if (nfc_poll(uid, sizeof(uid))) {
        if (!card_held || strcasecmp(uid, last_uid) != 0) {
            strncpy(last_uid, uid, kUidLen - 1);
            last_uid[kUidLen - 1] = '\0';
            card_held = true;
            handle_card(uid);
        }
    } else {
        card_held = false;
        last_uid[0] = '\0';
    }
}
