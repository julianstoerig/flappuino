#pragma once
#include "err.h"
#include "twi.h"
#include "typedefs.h"
#include "ssd1306.h"

#define BUF_SIZE (WIDTH * PAGES)


typedef struct Bar Bar;
struct Bar {
    U08 x;
    U08 y;
    U08 gap;
};

typedef struct Player Player;
struct Player {
    F32 y;
    F32 v;
    F32 a;
};

typedef enum State State;
enum State {
    STATE_MENU,
    STATE_RUNNING,
    STATE_GAME_OVER,
};

U08 bar_gap(U08 t);
