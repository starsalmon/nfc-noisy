#pragma once

#include "mapping.h"

static const int kMaxVolTags = 12;

enum class VolumeTagDir { None, Up, Down };

void volume_tags_begin();

VolumeTagDir volume_tag_direction(const char *uid_hex);

bool volume_tags_add(VolumeTagDir dir, const char *uid_hex);
bool volume_tags_remove(VolumeTagDir dir, const char *uid_hex);

int volume_tags_count(VolumeTagDir dir);
const char *volume_tags_uid(VolumeTagDir dir, int index);

void volume_tags_dump();
