#include "wifi_ota.h"

#if defined(NFCNOISY_WIFI_OTA)

#if __has_include("secrets.h")
#include "secrets.h"
#else
#error "NFCNoisy Wi-Fi/OTA: copy include/secrets.example.h to include/secrets.h and set WIFI_SSID, WIFI_PASSWORD, OTA_PASSWORD"
#endif

#ifndef WIFI_SSID
#error "secrets.h must define WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#error "secrets.h must define WIFI_PASSWORD"
#endif
#ifndef OTA_PASSWORD
#error "secrets.h must define OTA_PASSWORD"
#endif

#include "web_config.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>

static const char *kMdnsHost = "nfcnoisy";
static volatile bool s_ota_active = false;
static bool s_ota_started = false;
static bool s_mdns_started = false;
static unsigned long s_next_wifi_retry_ms = 0;
static unsigned long s_last_wifi_log_ms = 0;

static const char *wifi_status_name(wl_status_t st) {
    switch (st) {
    case WL_IDLE_STATUS:
        return "IDLE";
    case WL_NO_SSID_AVAIL:
        return "NO_SSID";
    case WL_SCAN_COMPLETED:
        return "SCAN_DONE";
    case WL_CONNECTED:
        return "CONNECTED";
    case WL_CONNECT_FAILED:
        return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
        return "LOST";
    case WL_DISCONNECTED:
        return "DISCONNECTED";
    default:
        return "UNKNOWN";
    }
}

static void log_wifi_state(bool force) {
    const unsigned long now = millis();
    if (!force && (now - s_last_wifi_log_ms) < 5000) {
        return;
    }
    s_last_wifi_log_ms = now;

    const wl_status_t st = WiFi.status();
    if (st == WL_CONNECTED) {
        Serial.printf("Wi-Fi %s  IP %s  RSSI %d dBm  host %s.local\n", wifi_status_name(st),
                      WiFi.localIP().toString().c_str(), WiFi.RSSI(), kMdnsHost);
    } else {
        Serial.printf("Wi-Fi %s (NFC/audio still running)\n", wifi_status_name(st));
    }
}

static void setup_ota_services() {
    if (s_ota_started) {
        return;
    }

    if (!MDNS.begin(kMdnsHost)) {
        Serial.println("mDNS start failed");
    } else {
        s_mdns_started = true;
        Serial.printf("mDNS: http://%s.local\n", kMdnsHost);
    }

    ArduinoOTA.setHostname(kMdnsHost);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        s_ota_active = true;
        Serial.println("OTA start — pausing NFC and audio");
    });
    ArduinoOTA.onEnd([]() {
        s_ota_active = false;
        Serial.println("OTA end");
    });
    ArduinoOTA.onError([](ota_error_t err) {
        s_ota_active = false;
        Serial.printf("OTA error %u\n", (unsigned)err);
    });

    ArduinoOTA.begin();
    s_ota_started = true;
    Serial.println("ArduinoOTA ready");
    web_config_on_wifi_ready();
}

static void wifi_start_connect() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(kMdnsHost);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    s_next_wifi_retry_ms = millis() + 15000;
    log_wifi_state(true);
}

void wifi_ota_begin() {
    Serial.println("Wi-Fi/OTA: background connect (non-blocking)");
    wifi_start_connect();
}

void wifi_ota_loop() {
    if (s_ota_started) {
        ArduinoOTA.handle();
        web_config_loop();
    }

    const wl_status_t st = WiFi.status();
    if (st == WL_CONNECTED) {
        if (!s_ota_started) {
            setup_ota_services();
        }
        log_wifi_state(false);
        return;
    }

    log_wifi_state(false);

    const unsigned long now = millis();
    if (now >= s_next_wifi_retry_ms) {
        Serial.println("Wi-Fi retry...");
        WiFi.disconnect(false);
        wifi_start_connect();
        s_next_wifi_retry_ms = now + 30000;
    }
}

bool wifi_ota_active() { return s_ota_active; }

#else

void wifi_ota_begin() {}
void wifi_ota_loop() {}
bool wifi_ota_active() { return false; }

#endif
