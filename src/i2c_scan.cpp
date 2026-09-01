#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 6
#define SCL_PIN 7

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