#pragma once

#define BAUD 19200
#define MYUBRR (F_CPU/16/BAUD - 1)

void UART_init(void);
uint8_t UART_getChar(void);
void UART_putChar(uint8_t c);
uint8_t UART_getString(uint8_t *buf);
void UART_putString(uint8_t *buf);

extern volatile uint8_t usart_int_occurred;
