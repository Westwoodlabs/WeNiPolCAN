#include <Arduino.h>
#include <Wire.h>

#include "net.h"
#include "wenipol.h"
#include "util.h"
#include "leds.h"
#include "wifi_setup.h"


const String logo_hex =
        "0000000002aaaa800000000000000000aa8002aa00000000"
        "0000002a80000002a8000000000002a0000000000a800000"
        "00000a000000000000a00000000028000000000000280000"
        "0000a00000000000000a0000000200000000000000008000"
        "000800000000000000002000002800000000000000002800"
        "00a000000000000000000a00028000000002800000000280"
        "0200000000028000000000800a00000000028000000000a0"
        "08000000000aa0000000002008000000000aa00000000020"
        "28000000000aa0000000002820000000002aa80000000008"
        "20000000002aa80000000008a0000800002aa8000020000a"
        "a000280000aaaa000028000a8000280000aaaa0000a80002"
        "8000aa0002aaaa8000aa00028000aa0002aaaa8000aa0002"
        "8002aa0002aaaa8002aa80028002aa800aaaaaa002aa8002"
        "800aaa800aaaaaa002aaa002a00aaa800aaaaaa002aaa00a"
        "a02aaaa02aaaaaa88aaaa80a202aaaa02aaaaaa80aaaa808"
        "20aaaaa02aaaaaa80aaaaa0828aaaaa8aaaaaaaa2aaaaa28"
        "08aaaaa8aaaaaaaa2aaaaa20082aaaa82aaaaaa82aaaa820"
        "0a2aaaaa2aaaaaa8aaaaa8a0020aaaaa2aaaaaa8aaaaa080"
        "028aaaaa0aaaaaa0aaaaa28000a2aaaa8aaaaaa2aaaa8a00"
        "0028aaaa8aaaaaa2aaaa280000082aaa8aaaaaa2aaa82000"
        "00020aaaa2aaaa8aaaa080000000a2aaa2aaaa8aaa8a0000"
        "0000282aa2aaaa8aa828000000000a0aa8aaaa2aa0a00000"
        "000002a028aaaa280a8000000000002a802aa802a8000000"
        "00000000aa8002aa000000000000000002aaaa8000000000";

uint8_t logo_bin[576];

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

//    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "0.nl.pool.ntp.org", "europe.pool.ntp.org", "time.nist.gov");
    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    init_wenipol();
    init_wifi();
    init_network();
    hex_decode(logo_hex, logo_bin, sizeof(logo_bin));

    #ifdef DEBUG
    logln("logo:");
    for (size_t i = 0; i < 12 * 48; i++) {
        print_byte(logo_bin[i]);
        if (i % 12 == 11) {
            logln();
        }
    }
    #endif

    tx_frame(1, logo_bin);
    show_frame(1, true, 200);
    xTaskCreate(wenipol_task, "WeNiPol Process", 6000, nullptr, 2, nullptr);

    #ifdef HAS_RGB_LED
    rgbIdle();
    #endif
}

void loop() {
    vTaskDelay(10000 / portTICK_PERIOD_MS);
}