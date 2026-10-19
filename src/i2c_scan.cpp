#include <Arduino.h>
#include <Wire.h>

#ifndef SCAN_SDA
#define SCAN_SDA 6
#endif
#ifndef SCAN_SCL
#define SCAN_SCL 7
#endif

#define SDA_PIN SCAN_SDA
#define SCL_PIN SCAN_SCL

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nI2C Scanner");

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);

    for (uint8_t address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.printf("Found device at 0x%02X\n", address);
        }
        else if (error == 4)
        {
            Serial.printf("Unknown error at 0x%02X\n", address);
        }
    }

    Serial.println("Done.");
}

void loop()
{
}