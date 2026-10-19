#pragma once

#include <Arduino.h>

#if defined(NFCNOISY_BOARD_FEATHERS3)
#include "boards/pins_feathers3.h"
#elif defined(NFCNOISY_BOARD_TINYC6)
#include "boards/pins_tinyc6.h"
#else
#include "boards/pins_esp32dev.h"
#endif
