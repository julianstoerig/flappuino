#pragma once
#include "typedefs.h"

#define WIDTH  (128)
#define HEIGHT ( 64)
#define PAGES (HEIGHT/8)
#define BUF_SIZE (WIDTH * PAGES)

typedef struct Rect Rect;
struct Rect {
    U08 x;
    U08 y;
    U08 w;
    U08 h;
};

typedef struct Bar Bar;
struct Bar {
    Rect upper;
    Rect lower;
};

typedef struct Player Player;
struct Player {
    Rect aabb;
    S08 v;
    S08 a;
};

typedef enum State State;
enum State {
    STATE_MENU,
    STATE_RUNNING,
    STATE_GAME_OVER,
};
