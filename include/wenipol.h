#pragma once
#include <Arduino.h>

namespace wenipol {

constexpr uint16_t max_brightness = 2400;

void init();
void tx_frame(uint8_t frame, uint8_t *frame_pixels);
void show_frame(uint8_t frame_id, boolean on, uint16_t brightness);
void show_gif(String& file_name);
void set_brightness(uint16_t new_brightness);

[[noreturn]] void background_task(void* parameter);

}

