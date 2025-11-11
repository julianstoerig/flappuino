#pragma once
#include "typedefs.h"

#define WIDTH  (128)
#define HEIGHT ( 64)
#define PAGES (HEIGHT/8)
#define BUF_SIZE (WIDTH * PAGES)


typedef struct Bar Bar;
struct Bar {
    U08 x;
    U08 y;
};

typedef struct Player Player;
struct Player {
    U08 y;
    S08 v;
};

typedef enum State State;
enum State {
    STATE_MENU,
    STATE_RUNNING,
    STATE_GAME_OVER,
};
