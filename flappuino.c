#include "flappuino.h"

#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>


#define BARS_LEN 6
#define BAR_GAP_MIN 16
#define BAR_GAP_MAX 32
#define BAR_WIDTH 5
#define BAR_SPACING (WIDTH / 3)

#define PLAYER_WIDTH 3
#define PLAYER_HEIGHT 3
#define PLAYER_X (WIDTH/3 - PLAYER_WIDTH*6)

#define MAX_SCORE 255

#define PLAYER_TAP_DELAY 1

static U16 prng_state = 1;

void prng_seed(U16 seed) {
    prng_state = seed ? seed : 1;
}

U08 prng_next(void) {
    prng_state = prng_state * 109 + 89;
    return prng_state >> 8;
}

U08 bar_gap(U08 t) {
    U08 res = BAR_GAP_MAX - (BAR_GAP_MIN*t)/(MAX_SCORE*WIDTH/3); // trend
    res +=  prng_next() % ((BAR_GAP_MAX-BAR_GAP_MIN)/2); // randomisation
    return res;
}

B08 player_collides_with_bar(Bar bar, Player player) {
    B08 to_the_left =  PLAYER_X + PLAYER_WIDTH < bar.x;
    B08 to_the_right = bar.x + BAR_WIDTH <= PLAYER_X;
    B08 above = player.y + PLAYER_HEIGHT < bar.y;
    B08 below = player.y > bar.y + bar.gap;
    B08 collides_with_bar = !(to_the_left | to_the_right) && (above | below);
    return collides_with_bar;
}

B08 game_over(Bar *bars, U08 len, Player player) {
    if (player.y < 0) return 1;
    if (player.y+PLAYER_HEIGHT > HEIGHT) return 1;
    for (U08 i=0; i<len; i+=1) {
        if (player_collides_with_bar(bars[i], player)) return 1;
    }
    return 0;
}

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

static U08 buf[BUF_SIZE] = {0};

static Bar bars[BARS_LEN];
static U08 bar_next = 0;
static Player player;
static U08 state;
static U08 score;
static F32 t;

void bar_fill(Bar *bar, U08 x) {
    bar->x = x;
    bar->gap = bar_gap(t);
    bar->y = prng_next() % (HEIGHT+1-bar->gap);
    if (bar->y+bar->gap >= HEIGHT) {
        bar->y = HEIGHT - 1 - bar->gap;
    }
}

void bars_init(Bar *bars, U08 len) {
    bar_next = 0;
    for (U08 i=0; i<len; i+=1) {
        bar_fill(bars+i, WIDTH*2/3 + i*BAR_SPACING);
    }
}

void game_reset(void) {
    score = 0;
    t = 0.0f;
    state = STATE_MENU;
    player = (Player) {
        .y = (F32)HEIGHT/2 - (F32)(PLAYER_HEIGHT+1)/2,
        .v = 0.0f,
    };
    bars_init(bars, BARS_LEN);
}

void draw_bar(Bar bar) {
#define BAR_END_OFFSET_X 1
#define BAR_END_OFFSET_Y 3

    draw_rect(buf, bar.x, 0, BAR_WIDTH, bar.y, 1); // top bar
    draw_rect(buf, bar.x, bar.y+bar.gap,
            BAR_WIDTH, HEIGHT-bar.y-bar.gap, 1); // bottom bar

    // set leftmost value of bar end
    U08 bar_end_x = 0;
    if (bar.x > BAR_END_OFFSET_X) {
        bar_end_x = bar.x - BAR_END_OFFSET_X;
    } else {
        bar_end_x = 0;
    }
    U08 bar_end_y = 0;
    if (bar.y >= BAR_END_OFFSET_Y) {
        bar_end_y = bar.y - BAR_END_OFFSET_Y;
    } else {
        bar_end_y = 0;
    }

    draw_rect(buf, bar_end_x, bar_end_y,
            BAR_WIDTH+BAR_END_OFFSET_X*2,
            BAR_END_OFFSET_Y, 1);

    bar_end_y = bar.y + bar.gap;

    draw_rect(buf, bar_end_x, bar_end_y,
            BAR_WIDTH+BAR_END_OFFSET_X*2,
            BAR_END_OFFSET_Y, 1);
}

int main(void) {
    prng_seed(1);

    twi_init();
    ssd1306_init();

    DDRB &= ~(1 << PB2);
    U08 button_down = 0;
    U08 button_old = button_down;
    U08 button_pressed = 0;

#define GAME_OVER_STR "game over"

    do {
        game_reset();
    } while (game_over(bars, BARS_LEN, player));

    while (1) {
        button_old = button_down;
        button_down = PINB & (1 << PB2);
        button_pressed = button_down && !button_old;

        t += 2;

        memset(buf, 0x00, BUF_SIZE);
        switch (state) {
            case STATE_MENU:
                draw_number(buf, WIDTH - 3*4, 0, score, 1);

                for (U08 i=0; i<BARS_LEN; i+=1) {
                    draw_bar(bars[i]);
                }

                draw_rect(buf, PLAYER_X, player.y, PLAYER_WIDTH, PLAYER_HEIGHT, 1);

                if (button_pressed) state = STATE_RUNNING;
                break;

            case STATE_GAME_OVER:
                memset(buf, 0xFF, BUF_SIZE);
                draw_number(buf, WIDTH/2-10, HEIGHT/2-3, score, 0);
                ssd1306_flip(buf);
                if (button_pressed) {
                    do {
                        game_reset();
                    } while (game_over(bars, BARS_LEN, player));
                }
                break;

            case STATE_RUNNING:
#define GRAVITY 0.35f
#define FLAP_STRENGTH -2.4f
#define MAX_VEL 4.5f
                if (button_pressed) {
                    player.v = FLAP_STRENGTH;
                }
                player.v += GRAVITY;
                if (player.v >  MAX_VEL) player.v =  MAX_VEL;

                player.y += 2*player.v;

                draw_number(buf, WIDTH - 3*4, 0, score, 1);

                for (U08 i=0; i<BARS_LEN; i+=1) {
                    // check if score needs updating
                    U08 bar_old_x = bars[i].x;
                    bars[i].x -= 2*1;
                    if (bar_old_x > PLAYER_X && bars[i].x <= PLAYER_X) {
                        if (score == MAX_SCORE) {
                            state = STATE_MENU;
                        } else {
                            score += 1;
                        }
                    }

                    draw_bar(bars[i]);

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
                if (game_over(bars, BARS_LEN, player)) {
                    for (U08 i=0; i<3; i+=1) {
                        ssd1306_flash_inverse();
                        _delay_ms(80);
                    }
                    state = STATE_GAME_OVER;
                }
                break;
        }

        ssd1306_flip(buf);
    }
}

