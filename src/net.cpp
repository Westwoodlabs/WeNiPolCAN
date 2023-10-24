#include "net.h"
#include "util.h"
#include "wenipol.h"
#include "ESPAsyncWebServer.h"
static AsyncWebServer server(80);


void init_network() {
    server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "OK Reboot");
        logln("REBOOTING");
        delay(100);
        ESP.restart();
    });

    server.on("^/frame/(\\d+)$", HTTP_PUT, [](AsyncWebServerRequest* request) -> void {
        uint8_t frame_id = request->pathArg(0).toInt();
        if (!request->hasArg("data")) {
            request->send(400, "text/plain", "Missing body");
            return;
        }
        auto frame_hex = request->arg("data");
        if (frame_hex.length() < 48 * 48 * 2 * 2 / 8) { // 48x48 pixels, 2 bits per pixel, 2 hex chars per byte
            request->send(400, "text/plain", "Invalid frame length");
            return;
        }

        uint8_t frame_bin[48 * 48 * 2 / 8];
        hex_decode(frame_hex, frame_bin, sizeof(frame_bin));
        tx_frame(frame_id, frame_bin);

        request->send(200, "text/plain", "OK");
    });

    server.on("^/frame/(\\d+)/:show$", HTTP_POST, [](AsyncWebServerRequest* request) -> void {
        uint8_t frame_id = request->pathArg(0).toInt();
        uint16_t brightness;
        if (request->hasArg("brightness")) {
            brightness = request->arg("brightness").toInt();
            if (!brightness)
                brightness = 200;
        } else {
            brightness = 200;
        }
        show_frame(frame_id, true, brightness);
        request->send(200, "text/plain", "OK");
    });

    server.begin();
}

void network_handler() {

}