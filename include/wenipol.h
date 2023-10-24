#pragma once
#include <Arduino.h>

void init_wenipol();
void tx_frame(uint8_t frame, uint8_t *frame_pixels);
void show_frame(uint8_t frame_id, boolean on, uint16_t brightness);
void wenipol_task(void* parameter);
