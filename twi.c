#include "twi.h"
#include "typedefs.h"
#include <util/delay.h>

void twi_init(void) {
    _delay_ms(200);

    TWSR = 0x00;
    TWBR = 12; // set bitrate TODO: make adjustable
    TWCR = 1 << TWEN;
}

B08 twi_start(void) {
    TWCR = 1 << TWEN | 1 << TWSTA | 1 << TWINT;
    twi_block_until_complete();
    return TW_STATUS != TW_START;
}

B08 twi_repeated_start(void) {
    TWCR = 1 << TWEN | 1 << TWSTA | 1 << TWINT;
    twi_block_until_complete();
    return TW_STATUS != TW_REP_START;
}

void twi_stop(void) {
    TWCR = 1 << TWEN | 1 << TWSTO | 1 << TWINT;
}

B08 twi_select_address_for_write(U08 address) {
    TWDR = (address << 1) & ~0x01;
    TWCR = 1 << TWEN | 1 << TWINT;
    twi_block_until_complete();
    return TW_STATUS != TW_MT_SLA_ACK;
}

B08 twi_select_address_for_read(U08 address) {
    TWDR = (address << 1) | 0x01;
    TWCR = 1 << TWEN | 1 << TWINT;
    twi_block_until_complete();
    return TW_STATUS != TW_MR_SLA_ACK;
}

B08 twi_write(U08 data) {
    TWDR = data;
    TWCR = 1 << TWEN | 1 << TWINT;
    twi_block_until_complete();
    return TW_STATUS != TW_MT_DATA_ACK;
}

S16 twi_write_array(U08 *data, S16 len) {
    S16 i = 0;
    for (; i<len; i+=1) {
        if (twi_write(data[i])) break;
    }
    return len - i;
}

B08 twi_read_ack(U08 *data) {
    TWCR = 1 << TWEN | 1 << TWEA | 1 << TWINT;
    twi_block_until_complete();
    *data = TWDR;
    return TW_STATUS != TW_MR_DATA_ACK;
}

B08 twi_read_noack(U08 *data) {
    TWCR = 1 << TWEN | 1 << TWINT;
    twi_block_until_complete();
    *data = TWDR;
    return TW_STATUS != TW_MR_DATA_NACK;
}

S16 twi_read_array(U08 *data, S16 cap, S16 *len, S16 n) {
    S16 i = 0;
    if (n > cap) n = cap;
    for (; i<n; i+=1) {
        if (twi_read_ack(data+i)) break;
    }
    if (!twi_read_noack(data+i)) i += 1;
    *len = i;
    return n - i;
}

