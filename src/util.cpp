#include "util.h"

static boolean serial_write_failed = false;

void set_serial_write_failed(boolean failed) {
    serial_write_failed = failed;
}

size_t hex_decode(const String &hex, uint8_t *buf, size_t len) {
    size_t skip = 0;
    for (size_t i = 0; i < len; i++) {
        char high_nibble = hex[2 * i + skip];
        char low_nibble = hex[2 * i + 1 + skip];
        if (high_nibble == '\0' || low_nibble == '\0') {
            return i;
        }
        if (high_nibble <= ' ' || low_nibble <= ' ') {
            skip++;
            i--;
            continue;
        }
        buf[i] = (high_nibble >= 'a' ? (high_nibble - 'a' + 10) :
                  high_nibble >= 'A' ? (high_nibble - 'A' + 10) : high_nibble - '0') << 4;
        buf[i] |= low_nibble >= 'a' ? (low_nibble - 'a' + 10) :
                  low_nibble >= 'A' ? (low_nibble - 'A' + 10) : low_nibble - '0';
    }
    return len;
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

boolean check_serial_write() {
    if (!Serial)
        return false;
    if (!Serial.availableForWrite() && serial_write_failed)
        return false;
    return true;
}