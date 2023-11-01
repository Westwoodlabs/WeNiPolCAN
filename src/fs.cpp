#include <LittleFS.h>
#include "FS.h"
#include "util.h"
#include "leds.h"
#include "fs.hpp"

void init_littlefs() {
    if (!LittleFS.begin(true)) {
        logln("Failed to mount LittleFS");
        showColorPattern(CRGB::Yellow, CRGB::Red, CRGB::Red);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
        ESP.restart();
    }
    auto fs = get_fs();
    if (!fs.exists("/")) {
        fs.mkdir("/");
    }
    if (!fs.exists("/gif")) {
        fs.mkdir("/gif");
    }
}

fs::FS &get_fs() {
    return LittleFS;
}
