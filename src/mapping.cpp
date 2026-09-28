#include "mapping.h"

#include "pins.h"

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <string.h>

struct MapEntry {
    char uid[kUidLen];
    char path[kPathLen];
};

static char setup_uid[kUidLen];
static MapEntry maps[kMaxMaps];
static int map_count = 0;
static char sounds[kMaxSounds][kPathLen];
static int sound_count = 0;

static const char *basename_of(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

static bool is_wav_name(const char *name) {
    const char *base = basename_of(name);
    const size_t n = strlen(base);
    if (n < 5) {
        return false;
    }
    return strcasecmp(base + n - 4, ".wav") == 0;
}

static void dump_dir(const char *path) {
    File dir = SD.open(path);
    if (!dir) {
        Serial.printf("SD open failed: %s\n", path);
        return;
    }
    Serial.printf("SD listing %s (%s)\n", path, dir.isDirectory() ? "dir" : "file");
    if (!dir.isDirectory()) {
        Serial.printf("  size %u\n", (unsigned)dir.size());
        dir.close();
        return;
    }
    dir.rewindDirectory();
    int n = 0;
    for (;;) {
        File f = dir.openNextFile();
        if (!f) {
            break;
        }
        char name[kPathLen];
        strncpy(name, f.name() ? f.name() : "?", kPathLen - 1);
        name[kPathLen - 1] = '\0';
        Serial.printf("  %s  %s  %u bytes\n", f.isDirectory() ? "DIR " : "FILE", name, (unsigned)f.size());
        f.close();
        n++;
        if (n >= 32) {
            Serial.println("  ...");
            break;
        }
    }
    dir.close();
}

void mapping_scan_sounds() {
    sound_count = 0;
    dump_dir("/");
    dump_dir(SOUNDS_DIR);

    File dir = SD.open(SOUNDS_DIR);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("No %s folder on SD\n", SOUNDS_DIR);
        return;
    }
    dir.rewindDirectory();

    for (;;) {
        File f = dir.openNextFile();
        if (!f) {
            break;
        }
        char name[kPathLen];
        strncpy(name, f.name() ? f.name() : "", kPathLen - 1);
        name[kPathLen - 1] = '\0';
        const bool skip = f.isDirectory() || name[0] == '.' || !is_wav_name(name);
        f.close();
        if (skip) {
            continue;
        }
        if (sound_count >= kMaxSounds) {
            Serial.println("Too many WAVs, first 16 used");
            break;
        }
        snprintf(sounds[sound_count], kPathLen, "%s/%s", SOUNDS_DIR, basename_of(name));
        sounds[sound_count][kPathLen - 1] = '\0';
        sound_count++;
    }
    dir.close();

    // Alphabetical so setup order is stable.
    for (int i = 0; i < sound_count; i++) {
        for (int j = i + 1; j < sound_count; j++) {
            if (strcasecmp(sounds[j], sounds[i]) < 0) {
                char tmp[kPathLen];
                strncpy(tmp, sounds[i], kPathLen);
                strncpy(sounds[i], sounds[j], kPathLen);
                strncpy(sounds[j], tmp, kPathLen);
            }
        }
    }

    Serial.printf("%d wav file(s) in %s\n", sound_count, SOUNDS_DIR);
    for (int i = 0; i < sound_count; i++) {
        Serial.printf("  %s\n", sounds[i]);
    }
}

bool mapping_load() {
    setup_uid[0] = '\0';
    map_count = 0;

    File f = SD.open(MAPPING_PATH, FILE_READ);
    if (!f) {
        Serial.println("No mapping.txt yet");
        mapping_scan_sounds();
        return true;
    }

    while (f.available() && map_count < kMaxMaps) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0 || line[0] == '#') {
            continue;
        }
        const int sp = line.indexOf(' ');
        if (sp <= 0) {
            continue;
        }
        String left = line.substring(0, sp);
        String right = line.substring(sp + 1);
        left.trim();
        right.trim();
        left.toUpperCase();
        if (left == "SETUP") {
            right.toUpperCase();
            strncpy(setup_uid, right.c_str(), kUidLen - 1);
            setup_uid[kUidLen - 1] = '\0';
            continue;
        }
        strncpy(maps[map_count].uid, left.c_str(), kUidLen - 1);
        strncpy(maps[map_count].path, right.c_str(), kPathLen - 1);
        maps[map_count].uid[kUidLen - 1] = '\0';
        maps[map_count].path[kPathLen - 1] = '\0';
        map_count++;
    }
    f.close();

    Serial.printf("Loaded mapping: setup=%s  %d card(s)\n", setup_uid[0] ? setup_uid : "(none)", map_count);
    mapping_scan_sounds();
    return true;
}

bool mapping_save() {
    File f = SD.open(MAPPING_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("Could not write mapping.txt");
        return false;
    }
    f.println("# NFCNoisy UID -> wav  (SETUP is the pairing card)");
    if (setup_uid[0]) {
        f.printf("SETUP %s\n", setup_uid);
    }
    for (int i = 0; i < map_count; i++) {
        f.printf("%s %s\n", maps[i].uid, maps[i].path);
    }
    f.close();
    Serial.println("Saved mapping.txt");
    return true;
}

int mapping_sound_count() { return sound_count; }

const char *mapping_sound_path(int index) {
    if (index < 0 || index >= sound_count) {
        return nullptr;
    }
    return sounds[index];
}

const char *mapping_setup_uid() { return setup_uid; }

void mapping_set_setup_uid(const char *uid_hex) {
    strncpy(setup_uid, uid_hex ? uid_hex : "", kUidLen - 1);
    setup_uid[kUidLen - 1] = '\0';
}

bool mapping_is_setup(const char *uid_hex) {
    return setup_uid[0] && uid_hex && strcasecmp(setup_uid, uid_hex) == 0;
}

const char *mapping_path_for_uid(const char *uid_hex) {
    if (!uid_hex) {
        return nullptr;
    }
    for (int i = 0; i < map_count; i++) {
        if (strcasecmp(maps[i].uid, uid_hex) == 0) {
            return maps[i].path;
        }
    }
    return nullptr;
}

void mapping_bind(const char *uid_hex, const char *path) {
    if (!uid_hex || !path) {
        return;
    }
    for (int i = 0; i < map_count; i++) {
        if (strcasecmp(maps[i].uid, uid_hex) == 0) {
            strncpy(maps[i].path, path, kPathLen - 1);
            maps[i].path[kPathLen - 1] = '\0';
            return;
        }
    }
    if (map_count >= kMaxMaps) {
        Serial.println("Mapping full");
        return;
    }
    strncpy(maps[map_count].uid, uid_hex, kUidLen - 1);
    strncpy(maps[map_count].path, path, kPathLen - 1);
    maps[map_count].uid[kUidLen - 1] = '\0';
    maps[map_count].path[kPathLen - 1] = '\0';
    map_count++;
}

void mapping_dump() {
    Serial.printf("SETUP %s\n", setup_uid[0] ? setup_uid : "(none)");
    for (int i = 0; i < map_count; i++) {
        Serial.printf("  %s -> %s\n", maps[i].uid, maps[i].path);
    }
}
