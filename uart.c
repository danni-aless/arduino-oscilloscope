#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

// REGISTRI UTILI
// n = 0,1,2,3 (four USARTs)
// UDRn = USART I/O Data Register n (RXB=Receive Data Buffer, TXB=Transmit Data Buffer)
// UCSRnA/B/C = USART Control and Status Registers
// UCSRnA:
// RXCn = USART Receive Complete (settato a 1 quando ci sono dati da leggere in RXB)
// UDREn = USART Data Register Empty (settato a 1 quando il TXB è pronto a ricevere nuovi dati da trasmettere)
// UCSRnB:
// RXCIEn = RX Complete Interrupt Enable n (abilita interrupt su RXCn)
// RXENn = Receiver Enable n (abilita l'USART Receiver)
// TXENn = Transmitter Enable n (abilita l'USART Transmitter)
// UCSRnC:
// UCSZn1:0 = Character Size (5 to 9-bit)
// UBRRnH/L = USART Baud Rate Registers

void UART_init(void) {
    // Imposta il baud rate
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)MYUBRR;
    // Asynchronous USART, parity mode disabled, 1 stop bit, 8-bit data
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    // Abilita interrupt su RXC0 e abilita l'USART Receiver e Transmitter
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
}

ISR(USART0_RX_vect) {
    usart_int_occurred = 1; // uso una flag così non uso la UART direttamente nella ISR
}

uint8_t UART_hasChar(void) {
    uint8_t c = 0;
    if(UCSR0A & (1<<RXC0))
        c = 1;
    return c;
}

uint8_t UART_getChar(void) {
    // Aspetta l'arrivo di nuovi dati
    while(!(UCSR0A & (1 << RXC0)));
    // Restituisci il dato
    return UDR0;
}

void UART_putChar(uint8_t c) {
    // Aspetta l'invio dei dati precedenti
    while(!(UCSR0A & (1 << UDRE0)));
    // Inizia a trasmettere
    UDR0 = c;
}

uint8_t UART_getString(uint8_t *buf) {
    uint8_t *b0 = buf;
    while(1) {
        uint8_t c = UART_getChar();
        *buf = c;
        ++buf;
        // leggere 0 fa terminare la stringa
        if(c == 0)
            return buf - b0;
        // leggere \n o \r fa terminare la stringa forzatamente
        if(c == '\n' || c == '\r') {
            *buf = 0;
            ++buf;
            return buf - b0;
        }
    }
}

void UART_putString(uint8_t *buf) {
    while(*buf) {
        UART_putChar(*buf);
        ++buf;
    }
}

// può essere migliorata per inserimento comandi multipli
uint8_t UART_getCommand(uint8_t *buf) {
    uint8_t *b0 = buf;
    while(buf-b0 < 3) {
        *buf = UART_getChar();
        ++buf;
    }
    return buf - b0;
}