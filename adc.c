#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"

// REGISTRI UTILI
// ADMUX = ADC Multiplexer Selection Register (REFS1:0, ADLAR, MUX4:0)
// REFS1:0 = Reference Selection Bits (AVcc = 5V)
// ADLAR = ADC Left Adjust Result (formato del risultato in ADCH/L)
// MUX4:0 = Analog Channel and Gain Selection Bits (input da usare)
// ADCSRA/B = ADC Control and Status Registers
// ADCSRA:
// ADEN = ADC Enable (abilita l'ADC)
// ADSC = ADC Start Conversion (1 -> inizia conversione, fine conversione -> 0)
// ADATE = ADC Auto Trigger Enable (abilita l'auto triggering)
// ADIE = ADC Interrupt Enable (abilita interrupt su ADC Conversion Complete)
// ADPS2:0 = ADC Prescaler Select Bits (2, 4, 8, 16, 32, 64, 128)
// 50-200kHz for 10-bit resolution, 50-1000kHz for 8-bit resolution
// ADCSRB:
// ADTS2:0 = ADC Auto Trigger Source (seleziona cosa triggera la conversione)
// ADCH/L = ADC Data Registers (memorizza il risultato, si legge prima ADCL e poi ADCH)

void ADC_init(void) {
    // Riferimento AVcc=5V, right-adjusted result, default input ADC0 
    ADMUX = 1 << REFS0;
    // Abilitato l'auto triggering e l'interrupt di terminata conversione
    ADCSRA = 1 << ADATE | 1 << ADIE;
    // Abilitato l'ADC e il prescaling a 2: (16MHz/64)/2 = 125kHz
    ADCSRA |= 1 << ADEN | 1 << ADPS0;
    // Timer/Counter1 Compare Match B triggera la conversione
    ADCSRB = 1 << ADTS2 | 1 << ADTS0;
}

ISR(ADC_vect) {
    adc_int_occurred = 1;
    current_value = ADCL + (ADCH << 8);
    current_channel = (current_channel+1)%total_channels;
    //ADMUX = 1 << REFS0 | current_channel;
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = 1 << REFS0 | channel;
    ADCSRA |= 1 << ADSC;
    while(ADCSRA & (1 << ADSC)); // busy waiting, vedere se si può implementare con interrupt
    return ADCL + (ADCH << 8);
}