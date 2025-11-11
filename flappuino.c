#include "flappuino.h"
#include <string.h>

static U16 prng_state = 1;

void prng_seed(U16 seed) {
    if (!seed) seed = 1;
    prng_state = seed;
}

U08 prng_next(void) {
    prng_state = prng_state * 2053 + 13849;
    return prng_state >> 8;
}

B08 aabb_point_collision(Rect a, U08 x, U08 y) {
    B08 r0 = a.x <= x;
    B08 r1 = a.y <= y;
    B08 r2 = a.x+a.w >= x;
    B08 r3 = a.y+a.h >= y;
    B08 res = r0 & r1 & r2 & r3;
    return res;
}

B08 aabb_aabb_collision(Rect a, Rect b) {
    B08 r0 = a.x + a.w < b.x;
    B08 r1 = b.x + b.w < a.x;
    B08 r2 = a.y + a.h < b.y;
    B08 r3 = b.y + b.h < a.y;
    B08 res = !(r0 | r1 | r2 | r3);
    return res;
}

B08 game_over(Bar *bars, U08 len, Rect player_aabb) {
    if (player_aabb.y < 0) return 1;
    if (player_aabb.y > HEIGHT) return 1;
    for (U08 i=0; i<len; i+=1) {
        if (aabb_aabb_collision(bars[i].upper, player_aabb)) return 1;
        if (aabb_aabb_collision(bars[i].lower, player_aabb)) return 1;
    }
    return 0;
}

#define F_CPU 16000000UL

// maybe incorrect
#define DISPLAY_ADDRESS 0x3C

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#include "twi.h"

#define SSD1306_DATA 0x40
#define SSD1306_COMMAND 0x00
#define SSD1306_CMD_DISPLAY_OFF 0xAE
#define SSD1306_CMD_DISPLAY_ON 0xAF
#define SSD1306_CMD_COL_RANGE 0x21
#define SSD1306_CMD_PAGE_RANGE 0x22

U08 font_digits[10][3] = {
    {0x1F, 0x11, 0x1F}, // 0
    {0x00, 0x00, 0x1F}, // 1
    {0x1D, 0x15, 0x17}, // 2
    {0x15, 0x15, 0x1F}, // 3
    {0x07, 0x04, 0x1F}, // 4
    {0x17, 0x15, 0x1D}, // 5
    {0x1F, 0x15, 0x1D}, // 6
    {0x01, 0x01, 0x1F}, // 7
    {0x1F, 0x15, 0x1F}, // 8
    {0x17, 0x15, 0x1F}  // 9
};

void setpixel(U08 *buf, U08 x, U08 y, B08 colour) {
    if (x >= WIDTH || y >= HEIGHT) return;
    U08 page = y / 8;
    U08 bit = y % 8; // TODO: check that avr-gcc optimises this
    U16 idx = page * WIDTH + x;
    if (colour) {
        buf[idx] |= (1 << bit);
    } else {
        buf[idx] &= ~(1 << bit);
    }
}

void draw_digit(U08 *buf, U08 x, U08 y, U08 digit, U08 colour) {
    if (digit > 9) return;
    for (U08 col=0; col<sizeof(font_digits[0]); col+=1) {
        U08 bits = font_digits[digit][col];
        for (U08 row=0; row<5; row+=1) {
            if (bits & (1 << row)) setpixel(buf, x+col, y+row, colour);
        }
    }
}

void draw_number(U08 *buf, U08 x, U08 y, B08 right_adjust, U08 number, U08 colour) {
    if (HEIGHT <= y) return;
#define MAX_DIGITS 3
    U08 digits[MAX_DIGITS] = {0};
    S08 i = 0;
    for (; i<MAX_DIGITS; i+=1) {
        digits[i] = number % 10; // TODO: check if avr-gcc optimises this
                                 // or find faster alternative
        number /= 10; // TODO: same as above
        if (!number) break;
    }
    if (right_adjust) {
        U08 n = i;
        x += (MAX_DIGITS-1)*(sizeof(font_digits[0])+1);
        for (; i>=0 && x+3<WIDTH; i-=1) {
            draw_digit(buf, x, y, digits[n-i], colour);
            x -= sizeof(font_digits[0]) + 1; // 1 for spacing
        }
    } else {
        for (; i>=0 && x+3<WIDTH; i-=1) {
            draw_digit(buf, x, y, digits[i], colour);
            x += sizeof(font_digits[0]) + 1; // 1 for spacing
        }
    }
}

void draw_hline(U08 *buf, U08 x, U08 y, U08 len, U08 colour) {
    U08 xmax = x + len;
    if (xmax >= WIDTH) xmax = WIDTH-1;
    for (U08 xcur=x; xcur<=xmax; xcur+=1) {
        setpixel(buf, xcur, y, colour);
    }
}

void draw_vline(U08 *buf, U08 x, U08 y, U08 len, U08 colour) {
    U08 ymax = y + len;
    if (ymax >= HEIGHT) ymax = HEIGHT-1;
    for (U08 ycur=y; ycur<=ymax; ycur+=1) {
        setpixel(buf, x, ycur, colour);
    }
}

void draw_rect(U08 *buf, Rect rect, U08 colour) {
    if (!rect.w | !rect.h) return;
    U08 ymax = rect.y + rect.h;
    if (ymax >= HEIGHT) ymax = HEIGHT - 1;
    for (U08 ycur=rect.y; ycur<=ymax; ycur+=1) {
        draw_hline(buf, rect.x, ycur, rect.w, colour);
    }
}

void ssd1306_init(void) {
    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_COMMAND);

    // fully initialise 128x64 display
    U08 init_cmds[] = {
        0xAE,           // 0xAE: set display off
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
        0xAF            // 0xAF: set display ON
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
    twi_write_array(buf, BUF_SIZE);
    twi_stop();
}

static U08 buf[BUF_SIZE] = {0};

#define BARS_LEN 6
Bar bars[BARS_LEN];
static Player player;
U08 state;
static U08 score;
static U08 t;

void bars_init(Bar *bars, U08 len) {
#define BAR_GAP 42
#define BAR_WIDTH 5
    for (U08 i=0; i<len; i+=1) {
        bars[i].upper.x = WIDTH + i * WIDTH/3;
        bars[i].lower.x = bars[i].upper.x;

        bars[i].upper.y = 0;
        bars[i].upper.h = prng_next() % (HEIGHT+1-BAR_GAP);
        bars[i].lower.y = bars[i].upper.h + BAR_GAP;
        bars[i].lower.h = HEIGHT - bars[i].lower.y;
        if (bars[i].lower.y >= HEIGHT) {
            bars[i].lower.y = HEIGHT - 1;
            bars[i].lower.h = 0;
        }
    }
}

void game_reset(void) {
    score = 0;
    t = 0;
    state = STATE_RUNNING;
    player = (Player) {
        .aabb.x = WIDTH/3,
        .aabb.y = HEIGHT/3,
        .aabb.w = 3,
        .aabb.h = 3,
        .v = 0,
    };
    bars_init(bars, BARS_LEN);
}

int main(void) {
    prng_seed(42);

    twi_init();
    ssd1306_init();

    DDRB &= ~(1 << PB2);
    U08 button_down = 0;
    U08 button_old = button_down;

#define GRAVITY 1
#define GAME_OVER_STR "game over"

    game_reset();

    while (1) {
        button_old = button_down;
        button_down = PINB & (1 << PB2);

        _delay_ms(10);
        t += 1;

        memset(buf, 0x00, BUF_SIZE);
        switch (state) {
            case STATE_MENU:
                break;

            case STATE_GAME_OVER:
                if (button_down && !button_old) game_reset();
                break;

            case STATE_RUNNING:
                if (game_over(bars, BARS_LEN, player.aabb)) {
                    state = STATE_GAME_OVER;
                    break;
                }
                player.v += -4*button_down*GRAVITY;
                player.v += GRAVITY;

                if (player.v < -4) player.v = -4;
                if (player.v >  4) player.v =  4;
                player.aabb.y += player.v;

                draw_number(buf, WIDTH - 3*4, 0, 1, score, 1);

                for (U08 i=0; i<BARS_LEN; i+=1) {
                    bars[i].upper.x -= 1;
                    bars[i].lower.x -= 1;
                    draw_rect(buf, bars[i].upper, 1);
                    draw_rect(buf, bars[i].lower, 1);
                }

                draw_rect(buf, player.aabb, 1);
                break;
        }

        ssd1306_flip(buf);
    }
}

