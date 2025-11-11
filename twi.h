#pragma once

#include <util/twi.h>
#include "typedefs.h"

#define twi_block_until_complete() while (!(TWCR & (1 << TWINT)))

void twi_init(void);

B08 twi_start(void);

void twi_stop(void);

B08 twi_select_address_for_write(U08 address);

B08 twi_select_address_for_read(U08 address);

B08 twi_write(U08 data);

S16 twi_write_array(U08 *data, S16 len);

B08 twi_read_ack(U08 *data);

B08 twi_read_noack(U08 *data);

S16 twi_read_array(U08 *data, S16 cap, S16 *len, S16 n);



