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
uint8_t first_byte, second_byte;

const uint8_t total_channels = 8;
uint8_t current_channel = 0;
uint8_t active_channels = 0; // bitmask
uint16_t sampling_freq = 1;

volatile uint8_t usart_int_occurred = 0;
volatile uint8_t adc_int_occurred = 0;
volatile uint8_t next_channel = 0;
volatile uint16_t current_value = 0;

int main(void) {
    UART_init();
    timer_init();
    ADC_init();
    sei();
    first_byte = 6<<5; // 6 -> message command
    second_byte = 0; // 0 -> initialization message
    UART_putChar(first_byte);
    UART_putChar(second_byte);
    while(1) {
        if(usart_int_occurred) {
            UART_getCommand(buf);
            if(buf[0]=='c' || buf[0]=='b') { 
                active_channels = buf[1];
            } else if(buf[0]=='f') {
                timer_updateSamplingFreq((buf[1] << 8) + buf[2]);
            } else if(buf[0]=='e' && buf[1]=='n' && buf[2]=='d') {
                active_channels = 0;
                first_byte = 7<<5; // 7 -> end command
                second_byte = 0;
                UART_putChar(first_byte);
                UART_putChar(second_byte);
            } else {
                first_byte = 6<<5; // 6 -> message command
                second_byte = 1; // 1 -> invalid command message
                UART_putChar(first_byte);
                UART_putChar(second_byte);
            }
            usart_int_occurred = 0;
        }
        if(adc_int_occurred) {
            current_channel = (next_channel-1)&7;
            if(active_channels & (1 << current_channel)) {
                first_byte = (current_channel<<2) | (current_value>>8);
                second_byte = current_value;
                UART_putChar(first_byte);
                UART_putChar(second_byte);
            }
            adc_int_occurred = 0;
        }
        //set_sleep_mode(SLEEP_MODE_IDLE);
        //sleep_mode();
    }
}