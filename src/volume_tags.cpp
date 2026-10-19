#include "volume_tags.h"

#include <Arduino.h>
#include <Preferences.h>
#include <string.h>

static Preferences s_prefs;
static char s_up[kMaxVolTags][kUidLen];
static char s_down[kMaxVolTags][kUidLen];
static int s_up_count = 0;
static int s_down_count = 0;

static void normalize_uid(char *dst, const char *uid_hex) {
    strncpy(dst, uid_hex ? uid_hex : "", kUidLen - 1);
    dst[kUidLen - 1] = '\0';
    for (char *p = dst; *p; p++) {
        if (*p >= 'a' && *p <= 'f') {
            *p = (char)(*p - 'a' + 'A');
        }
    }
}

static int find_uid(char list[][kUidLen], int count, const char *uid_hex) {
    for (int i = 0; i < count; i++) {
        if (strcasecmp(list[i], uid_hex) == 0) {
            return i;
        }
    }
    return -1;
}

static void remove_from(char list[][kUidLen], int *count, const char *uid_hex) {
    const int idx = find_uid(list, *count, uid_hex);
    if (idx < 0) {
        return;
    }
    for (int i = idx + 1; i < *count; i++) {
        strncpy(list[i - 1], list[i], kUidLen);
    }
    (*count)--;
    list[*count][0] = '\0';
}

static void save_list(const char *key_cnt, const char *key_prefix, char list[][kUidLen], int count) {
    s_prefs.putUChar(key_cnt, (uint8_t)count);
    for (int i = 0; i < kMaxVolTags; i++) {
        char key[8];
        snprintf(key, sizeof(key), "%s%d", key_prefix, i);
        if (i < count) {
            s_prefs.putString(key, list[i]);
        } else {
            s_prefs.remove(key);
        }
    }
}

static void load_list(const char *key_cnt, const char *key_prefix, char list[][kUidLen], int *count) {
    *count = (int)s_prefs.getUChar(key_cnt, 0);
    if (*count > kMaxVolTags) {
        *count = kMaxVolTags;
    }
    for (int i = 0; i < kMaxVolTags; i++) {
        list[i][0] = '\0';
    }
    for (int i = 0; i < *count; i++) {
        char key[8];
        snprintf(key, sizeof(key), "%s%d", key_prefix, i);
        String v = s_prefs.getString(key, "");
        normalize_uid(list[i], v.c_str());
    }
}

void volume_tags_begin() {
    s_prefs.begin("nfcnoisy", false);
    load_list("up_cnt", "up", s_up, &s_up_count);
    load_list("dn_cnt", "dn", s_down, &s_down_count);
    volume_tags_dump();
}

VolumeTagDir volume_tag_direction(const char *uid_hex) {
    if (!uid_hex || uid_hex[0] == '\0') {
        return VolumeTagDir::None;
    }
    if (find_uid(s_up, s_up_count, uid_hex) >= 0) {
        return VolumeTagDir::Up;
    }
    if (find_uid(s_down, s_down_count, uid_hex) >= 0) {
        return VolumeTagDir::Down;
    }
    return VolumeTagDir::None;
}

bool volume_tags_add(VolumeTagDir dir, const char *uid_hex) {
    if (!uid_hex || uid_hex[0] == '\0' || dir == VolumeTagDir::None) {
        return false;
    }
    char uid[kUidLen];
    normalize_uid(uid, uid_hex);

    remove_from(s_up, &s_up_count, uid);
    remove_from(s_down, &s_down_count, uid);

    if (dir == VolumeTagDir::Up) {
        if (s_up_count >= kMaxVolTags) {
            Serial.println("Volume up tag list full");
            return false;
        }
        strncpy(s_up[s_up_count], uid, kUidLen - 1);
        s_up_count++;
        save_list("up_cnt", "up", s_up, s_up_count);
    } else {
        if (s_down_count >= kMaxVolTags) {
            Serial.println("Volume down tag list full");
            return false;
        }
        strncpy(s_down[s_down_count], uid, kUidLen - 1);
        s_down_count++;
        save_list("dn_cnt", "dn", s_down, s_down_count);
    }
    Serial.printf("Volume tag %s -> %s\n", uid, dir == VolumeTagDir::Up ? "UP" : "DOWN");
    return true;
}

bool volume_tags_remove(VolumeTagDir dir, const char *uid_hex) {
    if (!uid_hex || dir == VolumeTagDir::None) {
        return false;
    }
    if (dir == VolumeTagDir::Up) {
        remove_from(s_up, &s_up_count, uid_hex);
        save_list("up_cnt", "up", s_up, s_up_count);
    } else {
        remove_from(s_down, &s_down_count, uid_hex);
        save_list("dn_cnt", "dn", s_down, s_down_count);
    }
    return true;
}

int volume_tags_count(VolumeTagDir dir) {
    if (dir == VolumeTagDir::Up) {
        return s_up_count;
    }
    if (dir == VolumeTagDir::Down) {
        return s_down_count;
    }
    return 0;
}

const char *volume_tags_uid(VolumeTagDir dir, int index) {
    if (dir == VolumeTagDir::Up) {
        if (index < 0 || index >= s_up_count) {
            return nullptr;
        }
        return s_up[index];
    }
    if (dir == VolumeTagDir::Down) {
        if (index < 0 || index >= s_down_count) {
            return nullptr;
        }
        return s_down[index];
    }
    return nullptr;
}

void volume_tags_dump() {
    Serial.printf("Volume tags: %d up, %d down\n", s_up_count, s_down_count);
    for (int i = 0; i < s_up_count; i++) {
        Serial.printf("  UP   %s\n", s_up[i]);
    }
    for (int i = 0; i < s_down_count; i++) {
        Serial.printf("  DOWN %s\n", s_down[i]);
    }
}
