#include <Arduino.h>
#include <Wire.h>
#include <mcp_can.h>

#define DEBUG

const char logo_hex[] =
        "0000000002aaaa800000000000000000aa8002aa00000000"
        "0000002a80000002a8000000000002a0000000000a800000"
        "00000a00000F000000a00000000028000000000000280000"
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


#define CAN0_INT 4
MCP_CAN CAN0(15);

constexpr uint32_t can_address_common = 0x1F0E1FFF;
constexpr uint32_t can_cmd_header     = 0x00B00000;
constexpr uint32_t can_cmd_data       = 0x00900000;
constexpr uint32_t can_cmd_trailer    = 0x00700000;
constexpr uint32_t can_address_show   = 0x1fc01fdf;

uint8_t can_clear_header[] { 0x00, 0x00, 0xFF, 0xFD, 0xFF, 0xFF, 0x7F, 0xF7};
uint8_t can_clear_payload1[] {0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
uint8_t can_clear_payloadn[] {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
uint8_t can_frame_header[] {0x01, 0x00, 0xFF, 0xFD, 0xFF, 0xFF, 0x7F, 0xF7};
uint8_t can_frame_trailer[] {0xFF, 0x7F};
uint8_t can_frame_show[] { 0x01, 0x00, 0xe0, 0x01, 0xee, 0xff, 0x00, 0x00};


void init_mcp();

void print_byte(uint8_t byte);
void hex_decode(const char *hex, uint8_t *buf, size_t len);

void encode_pixels(const uint8_t *pixels, uint8_t messages[8][8]);
void copy_segment(const uint8_t *pixels, uint8_t *segment, uint8_t x, uint8_t y);

void clear_segment(uint8_t x, uint8_t y);
void tx_segment(const uint8_t *segment_pixels, uint8_t x, uint8_t y);
void show_frame();
uint32_t get_can_address(uint8_t x, uint8_t y);

uint8_t logo_bin[576];
uint8_t msg_bytes[8][8];

void setup() {
    Serial.begin(115200);
    init_mcp();

    //send_message(msg_logo, sizeof(msg_logo) / sizeof(msg_logo[0]));
    uint8_t segment[4 * 16];
    hex_decode(logo_hex, logo_bin, sizeof(logo_bin));
#ifdef DEBUG
    Serial.println("logo:");
    for (size_t i = 0; i < 12 * 48; i++) {
        print_byte(logo_bin[i]);
        if (i % 12 == 11) {
            Serial.println("");
        }
    }
#endif
    for (size_t y = 0; y < 3; y++) {
        for (size_t x = 0; x < 3; x++) {
            copy_segment(logo_bin, segment, x, y);
            encode_pixels(segment, msg_bytes);
#ifdef DEBUG
            Serial.println("encoded:");
            for (size_t i = 0; i < 8; i++) {
                for (size_t j = 0; j < 8; j++) {
                    Serial.printf("%02X", msg_bytes[i][j]);
                }
                Serial.println();
            }
            Serial.println();
#endif
            clear_segment(x, y);
            tx_segment(segment, x, y);
        }
    }
}

void loop() {
    yield();
    delay(1000);
    show_frame();
}

void init_mcp() {
    // Initialize MCP2515 running at 8MHz with a baudrate of 1000kb/s and the
    // masks and filters disabled.
    if (CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
#ifdef DEBUG
        Serial.println("MCP2515 Initialized Successfully!");
#endif
        delay(1000);
    } else {
#ifdef DEBUG
        Serial.println("Error Initializing MCP2515...");
#endif
        delay(2000);
        setup();
    }

    CAN0.setMode(MCP_NORMAL); // Set operation mode to MCP_NORMAL so the MCP2515
    // sends acks to received data.

    pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
#ifdef DEBUG
    Serial.println("MCP2515 Ready...");
#endif
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

void copy_segment(const uint8_t *pixels, uint8_t *segment, uint8_t seg_x, uint8_t seg_y) {
    // copies one 16x16 pixel segment from the pixel data.
    // pixel data is 2bpp (red / yellow) -> 4 byte per row.
    for (size_t i = 0; i < 16; i++) { // copy one row at a time
        // as the whole panel is 3x3 segments, and each segment-row is 4 bytes,
        // we need to offset 12 bytes per panel-row.
        memcpy(&segment[i * 4], &pixels[(seg_y * 12 * 16) + (seg_x * 4) + (i * 12)], 4);
    }
}

void clear_segment(uint8_t x, uint8_t y) {
#ifdef DEBUG
    Serial.printf("clearing segment %d/%d\n", x, y);
#endif
    auto segment_address = get_can_address(x, y);
    CAN0.sendMsgBuf(segment_address | can_cmd_header, 1, 8, can_clear_header);
    CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, can_clear_payload1);
    for (int i = 0; i < 7; ++i) {
        CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, can_clear_payloadn);
    }
    CAN0.sendMsgBuf(segment_address | can_cmd_trailer, 1, 2, can_frame_trailer);
}

void tx_segment(const uint8_t *segment_pixels, uint8_t x, uint8_t y) {
#ifdef DEBUG
    Serial.printf("transmitting segment %d/%d\n", x, y);
#endif
    uint8_t messages[8][8];
    encode_pixels(segment_pixels, messages);
    auto segment_address = get_can_address(x, y);
    CAN0.sendMsgBuf(segment_address | can_cmd_header, 1, 8, can_frame_header);
    for (auto& message : messages) {
        CAN0.sendMsgBuf(segment_address | can_cmd_data, 1, 8, message);
    }
    CAN0.sendMsgBuf(segment_address | can_cmd_trailer, 1, 2, can_frame_trailer);
}

void show_frame() {
#ifdef DEBUG
    Serial.println("showing frame");
#endif
    CAN0.sendMsgBuf(can_address_show, 1, 8, can_frame_show);
}

uint32_t get_can_address(uint8_t x, uint8_t y) {
    // incrementing panel-id is encoded in bits 15-18 (from the left, 0-indexed) and inverted
    return can_address_common | (0xF ^ (y * 3 + x + 1)) << 13;
}

void hex_decode(const char *hex, uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        char high_nibble = hex[2 * i];
        char low_nibble = hex[2 * i + 1];
        buf[i] = (high_nibble >= 'a' ? (high_nibble - 'a' + 10) :
                  high_nibble >= 'A' ? (high_nibble - 'A' + 10) : high_nibble - '0') << 4;
        buf[i] |= low_nibble >= 'a' ? (low_nibble - 'a' + 10) :
                  low_nibble >= 'A' ? (low_nibble - 'A' + 10) : low_nibble - '0';
    }
}

const char *bit_rep[16] = {
        [0] = "0000", [1] = "0001", [2] = "0010", [3] = "0011",
        [4] = "0100", [5] = "0101", [6] = "0110", [7] = "0111",
        [8] = "1000", [9] = "1001", [10] = "1010", [11] = "1011",
        [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(uint8_t byte) {
    Serial.printf("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}