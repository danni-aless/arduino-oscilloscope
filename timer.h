#pragma once

void timer_init(void);
void timer_changeSamplingFreq(uint16_t freq);

extern const uint8_t total_channels;
extern uint16_t sampling_freq;