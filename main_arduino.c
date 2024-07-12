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
uint8_t current_channel = 0;
uint8_t active_channels = 0; // bitmask
uint16_t sampling_freq = 50;

uint8_t mode = 'c'; // (c)ontinuous or (b)uffered
uint16_t buffered_samples[MAX_BUF];
uint16_t buffered_index = 0;

volatile uint8_t usart_int_occurred = 0;
volatile uint8_t adc_int_occurred = 0;
volatile uint8_t next_channel = 0;
volatile uint16_t current_value = 0;

// send stored samples and reset index
void sendBulk(void) {
    for(int i=0; i<buffered_index; i++) {
        UART_sendCommand(buffered_samples[i]>>8, buffered_samples[i]&255);
        buffered_samples[i] = 0;
    }
    buffered_index = 0;
}

// store sample and update index
void storeSample(uint8_t first_byte, uint8_t second_byte) {
    buffered_samples[buffered_index] = (first_byte << 8) | second_byte;
    buffered_index++;
    if(buffered_index>=MAX_BUF) // if array reached max capacity send bulk
        sendBulk();
}

int main(void) {
    UART_init();
    timer_init();
    ADC_init();
    sei();
    UART_sendCommand(6<<5, 0); // 6.0 -> initialization message
    while(1) {
        if(usart_int_occurred) {
            UART_getCommand(buf);
            if(buf[0]=='m' && (buf[1]=='c' || buf[1]=='b')) {
                if(mode=='b' && buf[1]=='c') // when changing mode from buffered to continuous...
                    sendBulk(); // ...force sending bulk
                mode = buf[1];
                UART_sendCommand(6<<5, 2); // 6.2 -> mode updated message
            } else if(buf[0]=='c') { 
                active_channels = buf[1];
                UART_sendCommand(6<<5, 3); // 6.3 -> channels updated message
            } else if(buf[0]=='f') {
                timer_updateSamplingFreq((buf[1] << 8) + buf[2]);
                UART_sendCommand(6<<5, 4); // 6.4 -> frequency updated message
            } else if(buf[0]=='e' && buf[1]=='n' && buf[2]=='d') {
                active_channels = 0;
                UART_sendCommand(7<<5, 0); // 7.0 -> end command
            } else {
                UART_sendCommand(6<<5, 1); // 6.1 -> invalid command message
            }
            usart_int_occurred = 0;
        }
        if(adc_int_occurred) {
            current_channel = (next_channel-1)&7;
            if(active_channels & (1 << current_channel)) {
                uint8_t first_byte = (0<<5) | (current_channel<<2) | (current_value>>8);
                uint8_t second_byte = current_value;
                if(mode=='c') // continuous mode -> send the sample immediately
                    UART_sendCommand(first_byte, second_byte); // 0 -> sampling command
                else if(mode=='b') // buffered mode -> store the sample locally
                    storeSample(first_byte, second_byte);
            }
            adc_int_occurred = 0;
        }
        //set_sleep_mode(SLEEP_MODE_IDLE);
        //sleep_mode();
    }
}