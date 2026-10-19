#pragma once

#include <stdint.h>

void volume_begin();
int volume_get_percent();
bool volume_set_percent(int percent);
bool volume_step_up();
bool volume_step_down();

// Scale PCM before I2S (0–100% user volume, capped internally to limit clipping).
int32_t volume_scale_i32(int32_t sample);
int volume_scale_amp(int amp16);

void volume_play_feedback();
