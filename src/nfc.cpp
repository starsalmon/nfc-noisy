#include "nfc.h"

#include "pins.h"

#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>

static Adafruit_PN532 nfc(PIN_PN532_SS, &SPI);
static bool nfc_ok = false;

bool nfc_begin() {
    if (PIN_PN532_RST >= 0) {
        pinMode(PIN_PN532_RST, OUTPUT);
        digitalWrite(PIN_PN532_RST, HIGH);
        delay(10);
        digitalWrite(PIN_PN532_RST, LOW);
        delay(20);
        digitalWrite(PIN_PN532_RST, HIGH);
        delay(100);
    }

    nfc.begin();
    const uint32_t ver = nfc.getFirmwareVersion();
    if (!ver) {
        Serial.println("PN532 not found (SPI)");
        return false;
    }

    Serial.printf("PN532 SPI  IC 0x%02lX  fw %lu.%lu\n", (unsigned long)((ver >> 24) & 0xFF),
                  (unsigned long)((ver >> 16) & 0xFF), (unsigned long)((ver >> 8) & 0xFF));
    nfc.SAMConfig();
    nfc_ok = true;
    return true;
}

bool nfc_poll(char *uid_hex, size_t uid_hex_size) {
    if (!nfc_ok || uid_hex == nullptr || uid_hex_size < 3) {
        return false;
    }
    uid_hex[0] = '\0';

    uint8_t uid[7];
    uint8_t uid_len = 0;
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_len, 100)) {
        return false;
    }
    if (uid_len == 0 || uid_len > 7) {
        return false;
    }

    size_t o = 0;
    for (uint8_t i = 0; i < uid_len && o + 2 < uid_hex_size; i++) {
        o += (size_t)snprintf(uid_hex + o, uid_hex_size - o, "%02X", uid[i]);
    }
    uid_hex[o] = '\0';
    return true;
}
