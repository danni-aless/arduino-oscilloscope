#include <stdio.h> //snprintf
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
uint16_t sampling_freq = 1;

volatile uint8_t usart_int_occurred = 0;
volatile uint8_t adc_int_occurred = 0;

volatile uint8_t current_channel = 0;
volatile uint16_t current_value = 0;

int main(void) {
    UART_init();
    timer_init();
    ADC_init();
    sei();
    UART_putString((uint8_t*)"Initialization done!\n");
    UART_putString((uint8_t *)"Write which channel to print (0:8)\n");
    while(1) {
        if(usart_int_occurred) {
            UART_getString(buf);
            uint8_t channel_to_read = strtol((const char *)buf, (char **)NULL, 10);
            uint16_t value = ADC_read(channel_to_read);
            snprintf((char *)buf, MAX_BUF, "%d\n", value);
            UART_putString(buf);
            usart_int_occurred = 0;
        }
        if(adc_int_occurred) {
            snprintf((char *)buf, MAX_BUF, "%d\n", current_value);
            UART_putString(buf);
            adc_int_occurred = 0;
        }
        //set_sleep_mode(SLEEP_MODE_IDLE);
        //sleep_mode();
    }
}