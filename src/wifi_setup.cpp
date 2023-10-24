#include <WiFi.h>
#include <WiFiManager.h>
#include "wifi_setup.h"
#include "util.h"
#include "leds.h"

void init_wifi() {
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    bool res;
    res = wm.autoConnect("WeNiPol setup");
    if (!res) {
        logln("Failed to connect");
        showColorPattern(CRGB::Yellow, CRGB::Red, CRGB::Red);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP.restart();
    }
    log("Connected! IP address: ");
    logln(WiFi.localIP());
}