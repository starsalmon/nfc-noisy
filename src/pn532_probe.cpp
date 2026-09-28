#include <Arduino.h>
#include <Wire.h>

#ifndef PROBE_SDA
#define PROBE_SDA 6
#endif
#ifndef PROBE_SCL
#define PROBE_SCL 7
#endif

static const int PIN_SDA = PROBE_SDA;
static const int PIN_SCL = PROBE_SCL;

static void dump_from(uint8_t addr, int n) {
    const uint8_t got = (uint8_t)Wire.requestFrom((int)addr, n);
    Serial.printf("  requestFrom %d -> got %u:", n, got);
    for (uint8_t i = 0; i < got; i++) {
        Serial.printf(" %02X", Wire.read());
    }
    Serial.println();
}

static void try_firmware(uint8_t addr) {
    Serial.printf("\nGetFirmwareVersion @ 0x%02X (no ready-wait, raw dump)\n", addr);

    Wire.beginTransmission(addr);
    const uint8_t ping = Wire.endTransmission();
    Serial.printf("  ping %u (0=ACK)\n", ping);
    if (ping != 0) {
        return;
    }

    // Standard host frame: GetFirmwareVersion (0x02)
    const uint8_t packet[] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
    Wire.beginTransmission(addr);
    Wire.write(packet, sizeof(packet));
    const uint8_t wr = Wire.endTransmission();
    Serial.printf("  cmd write %u (0=OK)\n", wr);

    delay(50);
    dump_from(addr, 1);
    delay(20);
    dump_from(addr, 16);
    delay(50);
    dump_from(addr, 16);
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\nPN532 I2C deep probe");
    Serial.printf("Board %s  SDA=%d SCL=%d\n", ARDUINO_BOARD, PIN_SDA, PIN_SCL);
    Serial.println("RSTPDN on the red module should be tied to 3.3V.");
    Serial.println("This is GetFirmwareVersion (D4 02). Alive = ready 01 then a D5 03 frame.");

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(50000);
    Wire.setTimeOut(2000);
    delay(50);
}

void loop() {
    Serial.println("\n--- scan 6/7 ---");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  ACK 0x%02X\n", addr);
        }
    }

    try_firmware(0x24);
    try_firmware(0x28);

    Serial.println("\n(repeat in 10s)\n");
    delay(10000);
}
