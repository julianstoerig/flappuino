#include "flappuino.h"
#include <string.h>

#define BARS_LEN 5
#define BAR_GAP 32
#define BAR_WIDTH 5
#define BAR_SPACING (WIDTH / 3)

#define PLAYER_X (WIDTH/3)
#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3

static U16 prng_state = 1;

void prng_seed(U16 seed) {
    if (!seed) seed = 1;
    prng_state = seed;
}

U08 prng_next(void) {
    prng_state = prng_state * 2053 + 13849;
    return prng_state >> 8;
}

B08 collision(Bar bar, Player player) {
    B08 r0 = bar.x + BAR_WIDTH < PLAYER_X;
    B08 r1 = PLAYER_X + PLAYER_WIDTH < bar.x;
    B08 r2 = 0 + bar.y < player.y;
    B08 r3 = bar.y+BAR_GAP + HEIGHT-bar.y < player.y;
    B08 r4 = player.y + PLAYER_HEIGHT < 0;
    B08 r5 = player.y + PLAYER_HEIGHT < bar.y+BAR_GAP;
    B08 res = !(r0 | r1 | r2 | r3 | r4 | r5);
    return res;
}

B08 game_over(Bar *bars, U08 len, Player player) {
    if (player.y < 0) return 1;
    if (player.y > HEIGHT) return 1;
    for (U08 i=0; i<len; i+=1) {
        if (collision(bars[i], player)) return 1;
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
#define SSD1306_CMD_DISPLAY_NORMAL 0xA6
#define SSD1306_CMD_DISPLAY_INVERSE 0xA7
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

void draw_number(U08 *buf, U08 x, U08 y, U08 number, U08 colour) {
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
    U08 n = i;
    x += (MAX_DIGITS-1)*(sizeof(font_digits[0])+1);
    for (; i>=0 && x+3<WIDTH; i-=1) {
        draw_digit(buf, x, y, digits[n-i], colour);
        x -= sizeof(font_digits[0]) + 1; // 1 for spacing
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

void draw_rect(U08 *buf, U08 x, U08 y, U08 w, U08 h, U08 colour) {
    U08 ymax = y + h;
    if (ymax >= HEIGHT) ymax = HEIGHT - 1;
    for (U08 ycur=y; ycur<=ymax; ycur+=1) {
        draw_hline(buf, x, ycur, w, colour);
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

void ssd1306_flash_inverse(void) {
    twi_start();
    twi_select_address_for_write(DISPLAY_ADDRESS);
    twi_write(SSD1306_COMMAND);
    twi_write(SSD1306_CMD_DISPLAY_INVERSE);
    _delay_ms(100);
    twi_write(SSD1306_CMD_DISPLAY_NORMAL);
}

static U08 buf[BUF_SIZE] = {0};

static Bar bars[BARS_LEN];
static U08 bar_next = 0;
static Player player;
static U08 state;
static U08 score;
static U32 t;

void bar_fill(Bar *bar, U08 x) {
    bar->x = x;
    bar->y = prng_next() % (HEIGHT+1-BAR_GAP);
    if (bar->y+BAR_GAP >= HEIGHT) {
        bar->y = HEIGHT - 1 - BAR_GAP;
    }
}

void bars_init(Bar *bars, U08 len) {
    bar_next = 0;
    for (U08 i=0; i<len; i+=1) {
        bar_fill(bars+i, WIDTH + i*BAR_SPACING);
    }
}

void game_reset(void) {
    score = 0;
    t = 0;
    state = STATE_MENU;
    player = (Player) {
        .y = HEIGHT/3,
        .v = 0,
    };
    bars_init(bars, BARS_LEN);
}

int main(void) {
    prng_seed(1);

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
                draw_number(buf, WIDTH - 3*4, 0, score, 1);

                for (U08 i=0; i<BARS_LEN; i+=1) {
                    draw_rect(buf, bars[i].x, 0, BAR_WIDTH, bars[i].y, 1);
                    draw_rect(buf, bars[i].x, bars[i].y+BAR_GAP, BAR_WIDTH, HEIGHT-bars[i].y-BAR_GAP, 1);
                }

                draw_rect(buf, PLAYER_X, player.y, PLAYER_WIDTH, PLAYER_HEIGHT, 1);

                if (button_down && !button_old) state = STATE_RUNNING;
                break;

            case STATE_GAME_OVER:
                memset(buf, 0xFF, BUF_SIZE);
                draw_number(buf, WIDTH/2-10, HEIGHT/2-3, score, 0);
                ssd1306_flip(buf);
                if (button_down && !button_old) game_reset();
                break;

            case STATE_RUNNING:
                if (game_over(bars, BARS_LEN, player)) {
                    for (U08 i=0; i<3; i+=1) {
                        ssd1306_flash_inverse();
                        _delay_ms(80);
                    }
                    state = STATE_GAME_OVER;
                    break;
                }
                player.v += -2*button_down*GRAVITY;
                player.v += GRAVITY;

                if (player.v < -4) player.v = -4;
                if (player.v >  4) player.v =  4;
                player.y += player.v;

                draw_number(buf, WIDTH - 3*4, 0, score, 1);

                for (U08 i=0; i<BARS_LEN; i+=1) {
                    // check if score needs updating
                    U08 bar_old_x = bars[i].x;
                    bars[i].x -= 1;
                    if (bar_old_x > PLAYER_X && bars[i].x <= PLAYER_X)
                        score += 1;

                    // draw bar
                    draw_rect(buf, bars[i].x, 0, BAR_WIDTH, bars[i].y, 1);
                    draw_rect(buf, bars[i].x, bars[i].y+BAR_GAP, BAR_WIDTH, HEIGHT-bars[i].y-BAR_GAP, 1);

                    // check if bar has gone off screen
                    if (bars[bar_next].x + BAR_WIDTH == 0) {
                        U08 rightmost_bar_idx = bar_next + BARS_LEN - 1;
                        if (rightmost_bar_idx >= BARS_LEN) rightmost_bar_idx = 0;
                        U08 rightmost_x = bars[rightmost_bar_idx].x;
                        bar_fill(bars + bar_next, rightmost_x + BAR_SPACING);
                        bar_next += 1;
                        if (bar_next >= BARS_LEN) bar_next = 0;
                    }
                }

                draw_rect(buf, PLAYER_X, player.y, PLAYER_WIDTH, PLAYER_HEIGHT, 1);
                break;
        }

        ssd1306_flip(buf);
    }
}

