#include <Arduino.h>

static const int PIN_RX = 17;
static const int PIN_TX = 16;

static const uint8_t kWake[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
static const uint8_t kFw[] = {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
static const uint8_t kAck[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};

static const uint32_t kBauds[] = {9600, 19200, 38400, 57600, 115200};

static void drain(uint16_t ms) {
    uint32_t t0 = millis();
    while (millis() - t0 < ms) {
        while (Serial2.available()) {
            Serial2.read();
        }
        delay(2);
    }
}

static int read_n(uint8_t *buf, int maxn, uint16_t timeout_ms) {
    int n = 0;
    uint32_t t0 = millis();
    while (n < maxn && millis() - t0 < timeout_ms) {
        if (Serial2.available()) {
            buf[n++] = (uint8_t)Serial2.read();
            t0 = millis();
        } else {
            delay(2);
        }
    }
    return n;
}

static void print_buf(const char *label, const uint8_t *buf, int n) {
    Serial.printf("%s (%d):", label, n);
    for (int i = 0; i < n; i++) {
        Serial.printf(" %02X", buf[i]);
    }
    if (n == 0) {
        Serial.print(" (none)");
    }
    Serial.println();
}

static bool find_pat(const uint8_t *hay, int n, const uint8_t *needle, int m) {
    if (n < m) {
        return false;
    }
    for (int i = 0; i <= n - m; i++) {
        if (memcmp(hay + i, needle, (size_t)m) == 0) {
            return true;
        }
    }
    return false;
}

static void try_baud(uint32_t baud) {
    Serial.printf("\n=== baud %lu ===\n", (unsigned long)baud);
    Serial2.end();
    delay(50);
    Serial2.setRxBufferSize(256);
    Serial2.begin(baud, SERIAL_8N1, PIN_RX, PIN_TX);
    delay(200);
    drain(80);

    Serial2.write(kWake, sizeof(kWake));
    Serial2.flush();
    delay(150);
    drain(80);

    Serial2.write(kFw, sizeof(kFw));
    Serial2.flush();
    delay(30);

    uint8_t buf[80];
    const int n = read_n(buf, (int)sizeof(buf), 800);
    print_buf("rx", buf, n);

    const bool ack = find_pat(buf, n, kAck, 6);
    bool fw = false;
    for (int i = 0; i + 1 < n; i++) {
        if (buf[i] == 0xD5 && buf[i + 1] == 0x03) {
            fw = true;
        }
    }
    Serial.printf("  ACK %s  D5 03 %s\n", ack ? "YES" : "no", fw ? "YES" : "no");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\nPN532 HSU slow baud sweep");
    Serial.printf("ESP RX=%d TX=%d\n", PIN_RX, PIN_TX);
    for (uint8_t i = 0; i < sizeof(kBauds) / sizeof(kBauds[0]); i++) {
        try_baud(kBauds[i]);
        delay(400);
    }
    Serial.println("\nsweep done; repeats in 15s");
}

void loop() {
    delay(15000);
    setup();
}
