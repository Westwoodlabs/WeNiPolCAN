#pragma once
#include <Arduino.h>

template<typename... T> inline void log(T... args) {
#ifdef DEBUG
    if (Serial) {
        Serial.print(args...);
    }
#endif
}

template<typename... T> inline void logln(T... args) {
#ifdef DEBUG
    if (Serial) {
        Serial.println(args...);
    }
#endif
}

template<typename... T> inline void logf(T... args) {
#ifdef DEBUG
    if (Serial) {
        Serial.printf(args...);
    }
#endif
}

size_t hex_decode(const String &hex, uint8_t *buf, size_t len);
void print_byte(uint8_t byte);
