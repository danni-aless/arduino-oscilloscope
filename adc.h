#pragma once

void ADC_init(void);

extern const uint8_t total_channels;

extern volatile uint8_t adc_int_occurred;
extern volatile uint8_t next_channel;
extern volatile uint16_t current_value;