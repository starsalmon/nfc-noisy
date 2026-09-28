#pragma once

bool audio_begin();
void audio_beep(int freq_hz, int duration_ms, int amp16 = 1800);
void audio_boot_post();
bool audio_play_wav(const char *path);
