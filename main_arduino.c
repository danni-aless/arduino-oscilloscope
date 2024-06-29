#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "uart.h"
#include "timer.h"
#include "adc.h"

#define MAX_BUF 256
uint8_t buf[MAX_BUF];

const uint8_t total_channels = 8;
uint8_t active_channels = 0; // bitmask
uint16_t sampling_freq = 1;

volatile uint8_t usart_int_occurred = 0;
volatile uint8_t adc_int_occurred = 0;
volatile uint8_t next_channel = 0;
volatile uint16_t current_value = 0;

void stampaErrore(void) {
    snprintf((char *)buf, MAX_BUF, "Invalid command\n");
    UART_putString(buf);
}

int main(void) {
    UART_init();
    timer_init();
    ADC_init();
    sei();
    UART_putString((uint8_t*)"Initialization done!\n");
    while(1) {
        if(usart_int_occurred) {
            UART_getString(buf);
            if(buf[0]=='c') { 
                active_channels = buf[1];
            } else if(buf[0]=='f') {
                timer_updateSamplingFreq(strtol((const char *)buf+1, (char **)NULL, 10));
            } else {
                stampaErrore();
            }
            usart_int_occurred = 0;
        }
        if(adc_int_occurred) {
            if(active_channels & (1 << ((next_channel-1)&7))) {
                snprintf((char *)buf, MAX_BUF, "c%d=%d\n", (next_channel-1)&7, current_value);
                UART_putString(buf);
            }
            adc_int_occurred = 0;
        }
        //set_sleep_mode(SLEEP_MODE_IDLE);
        //sleep_mode();
    }
}