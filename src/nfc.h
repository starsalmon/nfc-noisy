#pragma once

#include <stddef.h>

bool nfc_begin();
// Returns true when a card is in the field. uid_hex is uppercase, no separators.
bool nfc_poll(char *uid_hex, size_t uid_hex_size);
