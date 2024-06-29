#pragma once

void ADC_init(void);
uint16_t ADC_read(uint8_t channel);

extern volatile uint8_t adc_int_occurred;
extern volatile uint8_t current_channel;
extern volatile uint16_t current_value;

extern const uint8_t total_channels;