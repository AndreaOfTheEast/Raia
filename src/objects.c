#include "../includes/objects.h"
#include "../includes/engine.h"
#include "../includes/utils.h"
#include "../includes/raymath.h"

#include <stdlib.h>
#include <math.h>

Objects objects;

void level_init(Level* level) {
    // Set up the level.
    level->dimensions = (Rectangle) {
        0, 0, 10, 10
    };
    level->size = level->dimensions.width * level->dimensions.height;
    level->map = (int *)calloc(level->size, sizeof(int));

    level->cellSize = 16;

    // Clear the level to just the boundaries.
    // DEBUG FOR NOW SHOULD LOAD VIA FILE.
    for (int x = 0; x < level->dimensions.width; x++) {
        level->map[x] = 1;
        level->map[(int)(x + (level->size - level->dimensions.width))] = 1;
    }
    for (int y = 0; y < level->dimensions.height; y++) {
        level->map[(int)(y * level->dimensions.width)] = 1;
        level->map[(int)(y * level->dimensions.width + level->dimensions.width - 1)] = 1;
    }

    level->map[25] = 1;
}

void level_deinit(Level* level) {
    free(level->map);
}

int level_getMapPos(Level* level, int x, int y) {
    if (x < 0 || x >= level->dimensions.width) return -1;
    if (y < 0 || y >= level->dimensions.height) return -1;
    return x + y * level->dimensions.width;
}

int level_getDataPoint(Level* level, int p) {
    if (p < 0 || p >= level->size) return -1;
    return level->map[p];
}

int level_getDataPos(Level* level, int x, int y) {
    return level_getDataPoint(level, level_getMapPos(level, x, y));
}

void level_update(Level* level) {

}

void level_draw(Level* level) {
    /*
    for (int y = 0; y < level->dimensions.height; y++) {
        for (int x = 0; x < level->dimensions.width; x++) {
            int p = level_getMapPos(level, x, y);
            if (p != -1) {
                Color color = GRAY;
                if (level_getDataPoint(level, p) != -1 && level_getDataPoint(level, p) > 0) color = WHITE;

                DrawRectangle(
                    x * level->cellSize,
                    y * level->cellSize,
                    level->cellSize -1,
                    level->cellSize - 1,
                    color
                );
            }
        }
    }
    */
}

void player_init(Player* player) {
    player->position = (Vector2) { objects.level.dimensions.width / 2, objects.level.dimensions.height / 2 };
    player->angle = PI * 2 - PI / 4;
    player->turnSpeed = 0.02f;
    player->speed = 0.07f;
}
void player_deinit(Player* player) {}
void player_update(Player* player) {
    // Turning
    if (IsKeyDown(KEY_A)) {
        player->angle -= player->turnSpeed;
    }
    if (IsKeyDown(KEY_D)) {
        player->angle += player->turnSpeed;
    }
    player->angle = clamp_angle(player->angle);

    // Foward/backward movement.
    Vector2 velocity = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_W)) {
        velocity.x = cos(player->angle) * player->speed;
        velocity.y = sin(player->angle) * player->speed;
    }
    if (IsKeyDown(KEY_S)) {
        velocity.x = -cos(player->angle) * player->speed;
        velocity.y = -sin(player->angle) * player->speed;
    }

    // Collisions
    if (level_getDataPos(
        &objects.level, 
        player->position.x + velocity.x, 
        player->position.y
    ) > 0) {
        velocity.x = 0.0f;
    }
    if (level_getDataPos(
        &objects.level, 
        player->position.x, 
        player->position.y + velocity.y
    ) > 0) {
        velocity.y = 0.0f;
    }


    // Apply movements.
    player->position = Vector2Add(player->position, velocity);
}
void player_draw(Player* player) {
    /*
    DrawCircle(
        player->position.x * objects.level.cellSize, 
        player->position.y * objects.level.cellSize, 
        4.0, BLUE
    );
    DrawLineEx(
        (Vector2) {
            (player->position.x * objects.level.cellSize),
            (player->position.y * objects.level.cellSize)
        }, 
        (Vector2) {
            (player->position.x * objects.level.cellSize) + cos(player->angle) * objects.level.cellSize,
            (player->position.y * objects.level.cellSize) + sin(player->angle) * objects.level.cellSize
        }, 
        2.0f,
        BLUE
    );*/

    // Raycaster
    raycast_observe(player->position, player->angle);
}

void objects_init() {
    level_init(&objects.level);
    player_init(&objects.player);
}

void objects_deinit() {
    player_deinit(&objects.player);
    level_deinit(&objects.level);
}

void objects_update() {
    level_update(&objects.level);
    player_update(&objects.player);
}

void objects_draw() {
    player_draw(&objects.player);
    level_draw(&objects.level);
}