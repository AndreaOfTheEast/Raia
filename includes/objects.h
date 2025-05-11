#ifndef OBJECTS_H
#define OBJECTS_H

#include "raylib.h"

// Object Structs

typedef struct Level {
    Rectangle dimensions;
    int size;
    int* map;

    int cellSize;
} Level;

typedef struct Player {
    Vector2 position;
    float angle;
    float turnSpeed;
    float speed;
} Player;

// Object Functions

void level_init(Level* level);
void level_deinit(Level* level);
int level_getMapPos(Level* level, int x, int y);
int level_getDataPoint(Level* level, int p);
int level_getDataPos(Level* level, int x, int y);
void level_update(Level* level);
void level_draw(Level* level);

void player_init(Player* player);
void player_deinit(Player* player);
void player_update(Player* player);
void player_draw(Player* player);

// Global objects container.

typedef struct Objects {
    Level level;
    Player player;
} Objects;

extern Objects objects;

void objects_init();
void objects_deinit();
void objects_update();
void objects_draw();

#endif