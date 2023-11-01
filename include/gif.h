#pragma once
#include "Arduino.h"
#include <vector>
#include <memory>

typedef struct frame {
    std::array<uint8_t, 576> pixels;
    uint16_t delay_ms;
} frame;

std::unique_ptr<std::vector<frame>> load_gif(String &path);