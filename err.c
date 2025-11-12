#include <avr/io.h>

#include "err.h"

void show_err(U08 code) {
    DDRC |= (1 << PC2);

    U08 len_bits = sizeof(code)*8;
    for (U08 i=0; i<len_bits; i+=1) {
        if ((code >> (len_bits - 1 - i)) & 0x01) {
            PORTC &= ~(1 << PC2);
            _delay_ms(50);
            PORTC |= (1 << PC2);
        } else {
            PORTC |= (1 << PC2);
            _delay_ms(50);
            PORTC &= ~(1 << PC2);
        }
        _delay_ms(500);
    }

    PORTC &= ~(1 << PC2);
}
