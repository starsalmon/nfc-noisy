#include "serial_config.h"

#include "nfc_session.h"
#include "volume.h"
#include "volume_tags.h"

#include <Arduino.h>
#include <string.h>

static char *trim(char *s) {
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    char *end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) {
        end--;
    }
    *end = '\0';
    return s;
}

static const char *arg_uid_or_last(const char *arg) {
    if (arg && arg[0] != '\0') {
        return arg;
    }
    const char *last = nfc_session_last_uid();
    if (last && last[0] != '\0') {
        return last;
    }
    return nullptr;
}

static void print_help() {
    Serial.println("Volume commands:");
    Serial.println("  vol              — show volume %");
    Serial.println("  vol <0-100>      — set volume");
    Serial.println("  vol last         — last scanned UID");
    Serial.println("  vol list         — list volume tags");
    Serial.println("  vol up [UID]     — assign tag as volume up (UID or last)");
    Serial.println("  vol down [UID]   — assign tag as volume down");
    Serial.println("  vol rm up UID    — remove volume up tag");
    Serial.println("  vol rm down UID  — remove volume down tag");
}

static void handle_line(char *line) {
    line = trim(line);
    if (line[0] == '\0') {
        return;
    }

    if (strcasecmp(line, "help") == 0 || strcasecmp(line, "?") == 0) {
        print_help();
        return;
    }

    if (strcasecmp(line, "vol") == 0 || strcasecmp(line, "volume") == 0) {
        Serial.printf("Volume %d%%\n", volume_get_percent());
        return;
    }

    if (strcasecmp(line, "vol last") == 0) {
        Serial.printf("Last UID: %s\n", nfc_session_last_uid()[0] ? nfc_session_last_uid() : "(none)");
        return;
    }

    if (strcasecmp(line, "vol list") == 0) {
        volume_tags_dump();
        return;
    }

    if (strncasecmp(line, "vol ", 4) == 0) {
        char *rest = trim(line + 4);
        if (strcasecmp(rest, "last") == 0) {
            Serial.printf("Last UID: %s\n", nfc_session_last_uid()[0] ? nfc_session_last_uid() : "(none)");
            return;
        }
        if (strcasecmp(rest, "list") == 0) {
            volume_tags_dump();
            return;
        }
        if (strncasecmp(rest, "up", 2) == 0) {
            char *up_args = trim((char *)(rest + 2));
            const char *uid = arg_uid_or_last(up_args);
            if (!uid) {
                Serial.println("No UID — tap a tag first or pass vol up <UID>");
                return;
            }
            volume_tags_add(VolumeTagDir::Up, uid);
            return;
        }
        if (strncasecmp(rest, "down", 4) == 0) {
            char *dn_args = trim((char *)(rest + 4));
            const char *uid = arg_uid_or_last(dn_args);
            if (!uid) {
                Serial.println("No UID — tap a tag first or pass vol down <UID>");
                return;
            }
            volume_tags_add(VolumeTagDir::Down, uid);
            return;
        }
        if (strncasecmp(rest, "rm ", 3) == 0) {
            char *rm = trim(rest + 3);
            if (strncasecmp(rm, "up ", 3) == 0) {
                volume_tags_remove(VolumeTagDir::Up, trim(rm + 3));
                return;
            }
            if (strncasecmp(rm, "down ", 5) == 0) {
                volume_tags_remove(VolumeTagDir::Down, trim(rm + 5));
                return;
            }
        }
        char *pct_arg = trim((char *)rest);
        const int pct = atoi(pct_arg);
        if (pct >= 0 && pct <= 100) {
            volume_set_percent(pct);
            Serial.printf("Volume %d%%\n", volume_get_percent());
            volume_play_feedback();
            return;
        }
    }

    Serial.println("Unknown command (try: help)");
}

void serial_config_poll() {
    if (!Serial.available()) {
        return;
    }
    static char buf[120];
    const int n = Serial.readBytesUntil('\n', buf, (int)sizeof(buf) - 1);
    if (n <= 0) {
        return;
    }
    buf[n] = '\0';
    handle_line(buf);
}
