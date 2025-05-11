#ifndef ENGINE_H
#define ENGINE_H

#include "raylib.h"

#define TEX_SIZE 4

static Color SAMPLE_TEX[TEX_SIZE * TEX_SIZE] = {
    WHITE, BLACK, WHITE, BLACK, 
    BLACK, WHITE, BLACK, WHITE,
    WHITE, BLACK, WHITE, BLACK, 
    BLACK, WHITE, BLACK, WHITE
};

typedef struct View {
    Rectangle dimensions;
    RenderTexture renderTexture;
} View;

typedef struct Window {
    Rectangle dimensions;
} Window;

typedef struct Engine {
    Window window;
    View view;

    int targetFPS;
} Engine;

extern Engine engine;

void engine_init();
void engine_deinit();

void engine_loop();
void engine_update();
void engine_draw();

// Raycasting Functions
float point_distance(Vector2 a, Vector2 b);

typedef struct Ray2D {
    Vector2 origin;
    Vector2 destination;
    float distance;
    int direction;
} Ray2D;

void ray2D_draw(Ray2D* ray2D);
Ray2D ray2D_castRay(Vector2 position, float angle);

void raycast_observe(Vector2 position, float angle);

#endif