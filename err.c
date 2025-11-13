#include <avr/io.h>
#include <avr/wdt.h>

#include "err.h"

void reboot(void) {
    wdt_enable(WDTO_15MS);
    for (;;);
}

void led_log(U08 code, B08 fast) {
    DDRC |= (1 << PC2);

    U08 len_bits = sizeof(code)*8;
    for (U08 i=0; i<len_bits; i+=1) {
        PORTC &= ~(1 << PC2);
        _delay_ms(100);
        PORTC |= (1 << PC2);
        _delay_ms(200);
        PORTC &= ~(1 << PC2);
        _delay_ms(100);
        if ((code >> (len_bits - 1 - i)) & 0x01) {
            PORTC |= (1 << PC2);
        } else {
            PORTC &= ~(1 << PC2);
        }
        if (fast) {
            _delay_ms(500);
        } else {
            _delay_ms(1000);
        }
    }

    PORTC &= ~(1 << PC2);
}

void led_err(U08 code, B08 fast) {
    led_log(code, fast);
    reboot();
}

void led_on(void) {
    PORTC |= (1 << PC2);
}

void led_off(void) {
    PORTC &= ~(1 << PC2);
}
