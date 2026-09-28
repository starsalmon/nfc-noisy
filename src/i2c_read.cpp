#include <Arduino.h>
#include <Wire.h>

// Exact test from git HEAD (30b9a80) — this is all that ever "responded".
#define SDA_PIN 6
#define SCL_PIN 7
#define ADDR 0x28

void setup() {
    Serial.begin(115200);
    delay(2000);

    Wire.begin(SDA_PIN, SCL_PIN);

    Serial.println("I2C read test (git original)");

    Wire.beginTransmission(ADDR);
    Wire.write(0x00);
    uint8_t e = Wire.endTransmission(false);

    Serial.printf("write: %d\n", e);

    uint8_t n = Wire.requestFrom(ADDR, 6);

    Serial.printf("read bytes: %d\n", n);

    while (Wire.available()) {
        Serial.printf("%02X ", Wire.read());
    }

    Serial.println();
}

void loop() {}
