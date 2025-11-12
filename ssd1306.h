#pragma once

#include <util/delay.h>

#include "typedefs.h"
#include "twi.h"

#define WIDTH  (128)
#define HEIGHT ( 64)
#define PAGES (HEIGHT/8)
#define DISPLAY_ADDRESS 0x3C

#define SSD1306_DATA 0x40
#define SSD1306_COMMAND 0x00
#define SSD1306_CMD_DISPLAY_NORMAL 0xA6
#define SSD1306_CMD_DISPLAY_INVERSE 0xA7
#define SSD1306_CMD_DISPLAY_OFF 0xAE
#define SSD1306_CMD_DISPLAY_ON 0xAF
#define SSD1306_CMD_COL_RANGE 0x21
#define SSD1306_CMD_PAGE_RANGE 0x22

void ssd1306_init(void);

void ssd1306_flip(U08 *buf);

void ssd1306_flash_inverse(void);

