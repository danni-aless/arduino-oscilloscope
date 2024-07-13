#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

// REGISTRI UTILI
// n = 1,3,4,5 (16-bit timers)
// x = A,B,C (output compare registers)
// TCCRnA/B/C = Timer/Counter Control Registers 
// TCCRnA:
// COMnx1:0 = Compare Outmode Mode for Channel x (inverting/non-inverting mode per Fast PWM)
// WGMn1:0 = Wave Generator Mode (PWM, CTC)
// TCCRnB:
// WGMn3:2 = Wave Generator Mode (PWM, CTC)
// CSn2:0 = Clock Select (prescaling: 1, 8, 64, 256, 1024)
// 
// TCNTn = Timer/Counter (TCNTH/L, 16-bit total)
// OCRnx = Output Compare Registers (OCRnxH/L, 16-bit total)
// TIMSKn = Timer/Counter n Interrupt Mask Register (OCIEnx)
// OCIEnx = Timer/Counter n, Output Compare x Match Interrupt Enable (TIMERn_COMPx_vect)

void timer_init(void)
{
    // Modalità CTC, prescaling a 64: 16MHz/64 = 250kHz
    TCCR1B = 1 << WGM12 | 1 << CS11 | 1 << CS10;
    // OCR1A per il reset del timer (CTC), OCR1B per l'auto triggering dell'ADC
    OCR1A = OCR1B = F_CPU / 64 / (total_channels*sampling_freq);
    // Abilitata l'interrupt di compare match 1B per l'auto triggering dell'ADC
    TIMSK1 = 1 << OCIE1B;
}

ISR(TIMER1_COMPB_vect) {
    // non serve fare nulla perché c'è l'auto triggering
}

void timer_updateSamplingFreq(uint16_t freq) {
    // controlli per limitare la frequenza in ingresso
    if(freq < 1)
        freq = 1;
    if(freq > 625)
        freq = 625;
    sampling_freq = freq;
    OCR1A = OCR1B = F_CPU / 64 / (total_channels*sampling_freq);
}