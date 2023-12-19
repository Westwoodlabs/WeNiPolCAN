#include <Arduino.h>
#include <Wire.h>
#include <rom/rtc.h>

#include "net.h"
#include "wenipol.h"
#include "fs.hpp"
#include "util.h"
#include "leds.h"
#include "wifi_setup.h"

void setup() {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    Serial.begin(115200);

    // starts the led task/state machine
    xTaskCreate(ledTask, "ledhandler", 2000, nullptr, 2, nullptr);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    // show a nice pattern to indicate the ESP is booting / waiting for WiFi setup
    showColorPattern(CRGB::Aqua, CRGB::Green, CRGB::Blue);

#ifdef BOARD_HAS_PSRAM
    if (!psramInit()) {
        log("This build of the AP expects PSRAM, but we couldn't find/init any. Something is terribly wrong here! System halted.");
        showColorPattern(CRGB::Yellow, CRGB::Red, CRGB::Red);
        while (1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    };
    heap_caps_malloc_extmem_enable(64);
#endif

    logln("\n\n##################################");
    logf("Internal Total heap %d, internal Free Heap %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
    logf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
    logf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n", ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
    logf("Flash Size %d, Flash Speed %d\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
    logln("##################################\n\n");

    logf("Total heap: %d\n", ESP.getHeapSize());
    logf("Free heap: %d\n", ESP.getFreeHeap());
    logf("Total PSRAM: %d\n", ESP.getPsramSize());
    logf("Free PSRAM: %d\n\n", ESP.getFreePsram());

    logf("ESP32 Partition table:\n");
    logf("| Type | Sub |  Offset  |   Size   |       Label      |\n");
    logf("| ---- | --- | -------- | -------- | ---------------- |\n");
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    if (pi != nullptr) {
        do {
            const esp_partition_t* p = esp_partition_get(pi);
            logf("|  %02x  | %02x  | 0x%06X | 0x%06X | %-16s |\r\n",
                 p->type, p->subtype, p->address, p->size, p->label);
        } while ((pi = (esp_partition_next(pi))));
    }

    init_littlefs();
    init_wifi();
    init_network();
    auto resetReason = rtc_get_reset_reason(0);
    if (resetReason == POWERON_RESET) {
        // if we just got powered on, the WeNiPol was likely just plugged in as well.
        // wait for the WeNiPol to initialize itself.
        logln("waiting 10s for WeNiPol to initialize itself...");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
    wenipol::init();

    xTaskCreate(wenipol::background_task, "WeNiPol Process", 64000, nullptr, 2, nullptr);

    #ifdef HAS_RGB_LED
    rgbIdle();
    #endif
}

void loop() {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
}