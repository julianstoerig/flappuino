#include "ssd1306.h"

void ssd1306_init(void) {
    _delay_ms(200);

    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_COMMAND);

    // fully initialise 128x64 display
    U08 init_cmds[] = {
        0xAE,           // 0xAE: set display off
        0xAF,           // 0xAE: set display on
        0xD5, 0x80,     // 0xD5: set display clock divide ratio/oscillator frequency, 0x80 (default)
        0xA8, HEIGHT-1, // 0xA8: set multiplex ratio, HEIGHT-1 (63 for 64-pix tall)
        0xD3, 0x00,     // 0xD3: set display offset, 0x00
        0x40,           // 0x40: set display start line to 0 (RAM address 0)
        0x8D, 0x14,     // 0x8D: charge pump setting, 0x14 (enable)
        0x20, 0x00,     // 0x20: set memory addressing mode, 0x00 (horizontal)
        0xA1,           // 0xA1: set segment re-map (0xA0 is normal, 0xA1 is column 0 mapped to SEG127)
        0xC8,           // 0xC8: set COM output scan direction (0xC0 is normal, 0xC8 is remapped)
        0xDA, 0x12,     // 0xDA: set COM pins hardware configuration, 0x12 (for 128x64)
        0x81, 0xCF,     // 0x81: set contrast control, 0xCF (max)
        0xD9, 0xF1,     // 0xD9: set pre-charge period, 0xF1
        0xDB, 0x40,     // 0xDB: set VCOM deselect level, 0x40 (0.83*VCC)
        0xA4,           // 0xA4: entire display OFF/ON (0xA4: normal, 0xA5: all ON)
        0xA6,           // 0xA6: normal display (0xA7: inverse display)
    };

    // Send all commands in one I2C transaction
    twi_write_array(init_cmds, sizeof(init_cmds));

    twi_stop();
}

void ssd1306_flip(U08 *buf) {
    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_COMMAND);

    //           v
    twi_write(0x00); // set low nibble of column start address to 0
    twi_write(0x10); // set high nibble of column start address to 0

    twi_write(0xB0); // set page start address

    twi_stop();

    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_DATA);
    twi_write_array(buf, WIDTH * PAGES);
    twi_stop();
}

void ssd1306_flash_inverse(void) {
    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_COMMAND);
    twi_write(SSD1306_CMD_DISPLAY_INVERSE);
    _delay_ms(100);
    twi_write(SSD1306_CMD_DISPLAY_NORMAL);
}


