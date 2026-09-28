#pragma once

#include <stddef.h>

static const int kMaxSounds = 16;
static const int kMaxMaps = 16;
static const int kPathLen = 80;
static const int kUidLen = 20;

bool mapping_load();
bool mapping_save();
void mapping_scan_sounds();

int mapping_sound_count();
const char *mapping_sound_path(int index);

const char *mapping_setup_uid();
void mapping_set_setup_uid(const char *uid_hex);
bool mapping_is_setup(const char *uid_hex);

const char *mapping_path_for_uid(const char *uid_hex);
void mapping_bind(const char *uid_hex, const char *path);

void mapping_dump();
