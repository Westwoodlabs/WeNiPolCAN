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

boolean check_serial_write() {
    if (!Serial)
        return false;
    if (!Serial.availableForWrite() && serial_write_failed)
        return false;
    return true;
}