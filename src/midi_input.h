#pragma once
#include <stdint.h>

void setup_midi_input();
void midi_in_task();
void process_midi_input(uint8_t b);
void handle_midi_event(uint8_t st, uint8_t d1, uint8_t d2);