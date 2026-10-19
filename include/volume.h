#pragma once

#include <stdint.h>

void volume_begin();
int volume_get_percent();
bool volume_set_percent(int percent);
bool volume_step_up();
bool volume_step_down();

// Scale PCM in native bit depth, then convert to 32-bit I2S slot (Philips / MAX98357A).
int32_t volume_pcm16_to_i32(int16_t pcm);
int32_t volume_pcm8_to_i32(int8_t pcm);  // unsigned 8-bit WAV: pass (byte - 128)
int volume_scale_amp(int amp16);

void volume_play_feedback();
