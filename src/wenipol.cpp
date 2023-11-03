#include "wenipol.h"
#include "util.h"
#include "leds.h"
#include "gif.h"
#include <mcp_can.h>
#include <vector>
#include <memory>

#define CAN0_INT 4
MCP_CAN CAN0(10);

constexpr uint32_t can_address_common = 0x1F0E1FFF;
constexpr uint32_t can_cmd_header     = 0x00B00000;
constexpr uint32_t can_cmd_data       = 0x00900000;
constexpr uint32_t can_cmd_trailer    = 0x00700000;
constexpr uint32_t can_address_show   = 0x1FC01FDF;

uint8_t can_frame_trailer[] {0xFF, 0x7F};

uint32_t get_can_address(uint8_t x, uint8_t y);
void encode_pixels(const uint8_t *pixels, uint8_t messages[8][8]);
void copy_segment(const uint8_t *pixels, uint8_t *segment, uint8_t seg_x, uint8_t seg_y);
void clear_segment(uint8_t frame, uint8_t x, uint8_t y);
void tx_segment(const uint8_t *segment_pixels, uint8_t frame, uint8_t x, uint8_t y);

void show_frame_internal(uint8_t frame_id, boolean on, uint16_t brightness);
void tx_frame_internal(uint8_t frame, uint8_t *frame_pixels, uint8_t show_frame);

static SemaphoreHandle_t can_mutex;
static String gif_path = "";
static boolean reload_gif = false;
static uint16_t brightness = 1200;

class CanLock {
public:
    bool success;
    explicit CanLock(uint32_t timeout_ms = 100) {
        success = xSemaphoreTake(can_mutex, timeout_ms * portTICK_PERIOD_MS);
    }

    ~CanLock() {
        if (success) {
            xSemaphoreGive(can_mutex);
        }
    }
};

void wenipol::init() {
    can_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(can_mutex);

    // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the
    // masks and filters disabled.
    if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
        logln("MCP2515 Initialized Successfully!");
    } else {
        logln("Error Initializing MCP2515...");
        showColorPattern(CRGB::Yellow, CRGB::Brown, CRGB::Red);
        while (1) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }

    CAN0.setMode(MCP_NORMAL); // Set operation mode to MCP_NORMAL so the MCP2515
    // sends acks to received data.

    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
    logln("MCP2515 Ready");
    logln("Init WeNiPol...");
    for (uint8_t x = 0; x < 3; x++) {
        for (uint8_t y = 0; y < 3; y++) {
            clear_segment(0, x, y);
        }
    }
    logln("Done");
}

void wenipol::tx_frame(uint8_t frame, uint8_t *frame_pixels) {
    CanLock lock(1000);
    if (!lock.success) {
        logln("tx_frame: unable to lock in 1000ms");
        return;
    }
    tx_frame_internal(frame, frame_pixels, frame);
}

void tx_frame_internal(uint8_t frame, uint8_t *frame_pixels, uint8_t show_frame) {
    uint8_t segment[4 * 16]; // 4 bytes per row, 16 rows
    for (size_t y = 0; y < 3; y++) {
        for (size_t x = 0; x < 3; x++) {
            copy_segment(frame_pixels, segment, x, y);
            // clear_segment(frame, x, y);
            tx_segment(segment, frame, x, y);
            show_frame_internal(show_frame, true, brightness);
        }
    }
}

void clear_segment(uint8_t frame, uint8_t x, uint8_t y) {
    logf("clearing segment %d/%d\n", x, y);
    auto segment_address = get_can_address(x, y);
    uint8_t can_clear_header[] { 0x00, 0x00, 0xFF, 0xFD, 0xFF, 0xFF, 0x7F, 0xF7};
    uint8_t can_clear_payload1[] {0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    uint8_t can_clear_payloadn[] {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
    CAN0.sendMsgBuf(segment_address | can_cmd_header, 1, 8, can_clear_header);
    CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, can_clear_payload1);
    for (int i = 0; i < 7; ++i) {
        CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, can_clear_payloadn);
    }
    CAN0.sendMsgBuf(segment_address | can_cmd_trailer, 1, 2, can_frame_trailer);
}

void tx_segment(const uint8_t *segment_pixels, uint8_t frame, uint8_t x, uint8_t y) {
    logf("transmitting frame %d segment %d/%d\n", frame, x, y);
    uint8_t can_frame_header[] {
        frame, 0x00, 0xFF, 0xFD, 0xFF, 0xFF, 0x7F, 0xF7};

    uint8_t messages[8][8];
    encode_pixels(segment_pixels, messages);
    auto segment_address = get_can_address(x, y);
    CAN0.sendMsgBuf(segment_address | can_cmd_header, 1, 8, can_frame_header);
    for (auto& message : messages) {
        CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, message);
    }
    CAN0.sendMsgBuf(segment_address | can_cmd_trailer, 1, 2, can_frame_trailer);
}

void wenipol::show_frame(uint8_t frame_id, boolean on, uint16_t brightness) {
    CanLock lock(1000);
    if (!lock.success) {
        logln("show_frame: unable to lock in 1000ms");
        return;
    }
    if (brightness > wenipol::max_brightness) {
        brightness = wenipol::max_brightness;
    }
    logf("showing frame %d, on=%d, brightness=%d\n", frame_id, on, brightness);
    show_frame_internal(frame_id, on, brightness);
}

void show_frame_internal(uint8_t frame_id, boolean on, uint16_t brightness) {
    uint8_t can_frame_show[] {
        frame_id,
        (uint8_t ) (on ? 0x00 : 0x0f),
        (uint8_t) (brightness & 0xFF),
        (uint8_t) (brightness >> 8),
        0xee, 0xff, 0x00, 0x00};
    CAN0.sendMsgBuf(can_address_show, 1, 8, can_frame_show);
}

uint32_t get_can_address(uint8_t x, uint8_t y) {
    // incrementing panel-id is encoded in bits 15-18 (from the left, 0-indexed) and inverted
    return can_address_common | (0xF ^ (y * 3 + x + 1)) << 13;
}

void copy_segment(const uint8_t *pixels, uint8_t *segment, uint8_t seg_x, uint8_t seg_y) {
    // copies one 16x16 pixel segment from the pixel data.
    // pixel data is 2bpp (red / yellow) -> 4 byte per row.
    for (size_t i = 0; i < 16; i++) { // copy one row at a time
        // as the whole panel is 3x3 segments, and each segment-row is 4 bytes,
        // we need to offset 12 bytes per panel-row.
        memcpy(&segment[i * 4], &pixels[(seg_y * 12 * 16) + (seg_x * 4) + (i * 12)], 4);
    }
}

void encode_pixels(const uint8_t *pixels, uint8_t messages[8][8]) {
    // px is xy pixel data, 2bit (red, yellow), 16 rows, 16 columns
    // => each row is compressed into 4 bytes (16 cols / 2 bits per pixel)
    // CAVEAT: the whole panel is rotated by 180 degrees, the following effectively describes
    //         the panel from bottom right to top left.
    // there are four groups ("A", "B", "C", "D"; using indexes 0-3)
    // each group has 4 bits per color and row (two colors: first 4 bits are red, second 4 bits are yellow)
    // each bit is every fourth LED in the physical row.
    //
    // to be concrete: each 32bit row of pixel data is converted into 4 bytes of CAN payload
    //                 according to the following bit mapping:
    // 32-bit (0-V)     0xb 0123456789ABCDEFGHIJKLMNOPQRSTUV
    //  8-bit (group 0) 0xb 6EMU7FNV
    //  8-bit (group 1) 0xb 4CKS5DLT
    //  8-bit (group 2) 0xb 2AIQ3BJR
    //  8-bit (group 3) 0xb 08GO19HP
    //
    //  rows are traversed in reverse order (bottom-up).
    //  two rows (8-byte) constitute one CAN payload.
    //  as we have 16 rows, we need 8 CAN payloads.
    size_t payload_index = 0;
    for (size_t group = 0; group < 4; group++) {
        size_t payload_byte_index = 0;
        for (size_t row_index = 16; row_index > 0; row_index--) {
            uint32_t row;
            memcpy(&row, &pixels[(row_index - 1) * 4], sizeof(uint32_t));

            uint8_t row_data = 0;

            // red 4-bits in current group & row_index
            row_data |= ((row & (0b10 << (3 * 8 + (group * 2)))) != 0) << 7; // move 0bx0 to 0bx0000000
            row_data |= ((row & (0b10 << (2 * 8 + (group * 2)))) != 0) << 6; // move 0bx0 to 0b0x000000
            row_data |= ((row & (0b10 << (1 * 8 + (group * 2)))) != 0) << 5; // move 0bx0 to 0b00x00000
            row_data |= ((row & (0b10 << (0 * 8 + (group * 2)))) != 0) << 4; // move 0bx0 to 0b000x0000
            // yellow 4-bits in current group  & row_index
            row_data |= ((row & (0b01 << (3 * 8 + (group * 2)))) != 0) << 3; // move 0b0x to 0b0000x000
            row_data |= ((row & (0b01 << (2 * 8 + (group * 2)))) != 0) << 2; // move 0b0x to 0b00000x00
            row_data |= ((row & (0b01 << (1 * 8 + (group * 2)))) != 0) << 1; // move 0b0x to 0b000000x0
            row_data |= ((row & (0b01 << (0 * 8 + (group * 2)))) != 0) << 0; // move 0b0x to 0b0000000x

            if (row_index == 8) {
                payload_index++;
                payload_byte_index = 0;
            }
            messages[payload_index][payload_byte_index++] = row_data;
        }
        payload_index++;
    }
}

void wenipol::show_gif(String& file_name) {
    if (gif_path.equals(file_name)) {
        logf("show_gif: already showing %s... reloading\n", file_name.c_str());
    }
    gif_path = file_name;
    reload_gif = true;
}

void wenipol::set_brightness(uint16_t new_brightness) {
    brightness = new_brightness;
}

[[noreturn]] void wenipol::background_task(void *parameter) {
    std::unique_ptr<std::vector<frame>> frames = nullptr;
    typeof(frames->begin()) current_frame = {};
    uint8_t write_frame = 1;
    uint8_t display_frame = 2;

    while (true) {
        if (reload_gif) {
            frames = load_gif(gif_path);
            if (!frames || frames->empty()) {
                logln("Failed to load gif");
            } else {
                current_frame = frames->begin();
            }
            reload_gif = false;
        }
        if (current_frame != typeof(frames->begin()) {}) {
            unsigned long render_start = millis();
            {
                CanLock lock(1000);
                if (!lock.success) {
                    logln("render_frame: unable to lock in 1000ms");
                } else {
                    tx_frame_internal(write_frame, current_frame->pixels.data(), display_frame);
                    auto tmp = write_frame; write_frame = display_frame; display_frame = tmp;
                    show_frame_internal(display_frame, true, brightness);
                }
            }
            unsigned long delay_millis = current_frame->delay_ms - (millis() - render_start);
            if (delay_millis > 1000000) {
                logf("bad delay: %lu\n", delay_millis);
            } else {
                vTaskDelay(delay_millis / portTICK_PERIOD_MS);
            }
            current_frame++;
            if (current_frame == frames->end()) {
                current_frame = frames->begin();
            }
        } else {
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}