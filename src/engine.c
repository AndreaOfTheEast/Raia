#include "../includes/engine.h"
#include "../includes/objects.h"
#include "../includes/raymath.h"
#include "../includes/utils.h"

#include <math.h>
#include <stdio.h>

Engine engine;

void engine_init() {
    engine.window.dimensions = (Rectangle) {
        0, 0, 3200, 2000
    };
    engine.targetFPS = 60;

    InitWindow(
        engine.window.dimensions.width, 
        engine.window.dimensions.height, 
        "Raia"
    );
    SetTargetFPS(engine.targetFPS);
    ToggleFullscreen();

    engine.view.dimensions = (Rectangle) {
        0, 0, 
        engine.window.dimensions.width / 4,
        engine.window.dimensions.height / 4
    };
    engine.view.renderTexture = LoadRenderTexture(
        engine.view.dimensions.width,
        engine.view.dimensions.height
    );

    objects_init();
}

void engine_deinit() {
    objects_deinit();
    CloseWindow();
}

void engine_loop() {
    while (!WindowShouldClose()) {
        engine_update();
        engine_draw();
    }
}

void engine_update() {
    objects_update();
}

void engine_draw() {
    BeginTextureMode(engine.view.renderTexture);
    ClearBackground(BLACK);
    objects_draw();
    DrawFPS(8, 8);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);

    Rectangle source = engine.view.dimensions;
    Rectangle dimension = engine.window.dimensions;
    source.height = -source.height;

    DrawTexturePro(
        engine.view.renderTexture.texture, 
        source,
        dimension, 
        (Vector2) {0, 0}, 0, WHITE);
    EndDrawing();
}

// Raycasting Functions

void ray2D_draw(Ray2D* ray2D) {
    DrawLineV(
        (Vector2) { ray2D->origin.x * objects.level.cellSize,  ray2D->origin.y * objects.level.cellSize }, 
        (Vector2) { ray2D->destination.x * objects.level.cellSize,  ray2D->destination.y * objects.level.cellSize },
        RED
    );
}

float point_distance(Vector2 a, Vector2 b) {
    return sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
}

Ray2D ray2D_castRay(Vector2 position, float angle) {
    angle = clamp_angle(angle);

    int DOF = 0;
    int DOFMax = 20;

    Ray2D ray;
    ray.origin = position;
    ray.destination = position;

    Vector2 rayDistance = {
        1e30,
        1e30
    };
    Vector2 horizontalRay = {
        position.x,
        position.y
    };
    Vector2 verticalRay = {
        position.x,
        position.y
    };
    Vector2 offset = {
        0.0f, 0.0f
    };

    float aTan = -1 / tan(angle);

    // Horizontal Line Check
    // Looking Up
    if (angle > PI) {
        ray.destination.y = floor(position.y) - 0.0001;
        ray.destination.x = (position.y - ray.destination.y) * aTan + position.x;
        offset.y = -1;
        offset.x = -offset.y * aTan;
    }
    // Looking Down
    if (angle < PI) {
        ray.destination.y = floor(position.y) + 1;
        ray.destination.x = (position.y - ray.destination.y) * aTan + position.x;
        offset.y = 1;
        offset.x = -offset.y * aTan;
    }
    // Looking Left or Right
    if (angle == 0 || angle == PI) {
        ray.destination.x = position.x;
        ray.destination.y = position.y;
        DOF = DOFMax;
    }

    // DDA
    while (DOF < DOFMax) {
        int p = level_getMapPos(&objects.level, (int)ray.destination.x, (int)ray.destination.y);
        if (p >= 0 && p < objects.level.size && objects.level.map[p] > 0) {
            horizontalRay = ray.destination;
            rayDistance.x = point_distance(position, horizontalRay);
            DOF = DOFMax;
        } else {
            ray.destination = Vector2Add(ray.destination, offset);
            DOF++;
        }
    }

    // Vertical Line Check
    float nTan = -tan(angle);
    DOF = 0;
    // Looking Left
    if (angle > PI / 2 && angle < PI / 2 * 3) {
        ray.destination.x = floor(position.x) - 0.0001;
        ray.destination.y = (position.x - ray.destination.x) * nTan + position.y;
        offset.x = -1;
        offset.y = -offset.x * nTan;
    }
    // Looking Right
    if (angle < PI / 2 || angle > PI / 2 * 3) {
        ray.destination.x = floor(position.x) + 1;
        ray.destination.y = (position.x - ray.destination.x) * nTan + position.y;
        offset.x = 1;
        offset.y = -offset.x * nTan;
    }
    // Looking Up or Down
    if (angle == 0 || angle == PI) {
        ray.destination.x = position.x;
        ray.destination.y = position.y;
        DOF = DOFMax;
    }

    // DDA
    while (DOF < DOFMax) {
        int p = level_getMapPos(&objects.level, (int)ray.destination.x, (int)ray.destination.y);
        if (p >= 0 && p < objects.level.size && objects.level.map[p] > 0) {
            verticalRay = ray.destination;
            rayDistance.y = point_distance(position, verticalRay);
            DOF = DOFMax;
        } else {
            ray.destination = Vector2Add(ray.destination, offset);
            DOF++;
        }
    } 

    // Adding Ray Data
    // Choose the smaller ray.

    if (rayDistance.y < rayDistance.x) {
        ray.destination = verticalRay;
        ray.direction = 1;
        
    } else if (rayDistance.x < rayDistance.y ){
        ray.destination = horizontalRay;
        ray.direction = 0;
    } else {
        ray.direction = -1;
    }

    ray.distance = point_distance(ray.origin, ray.destination);

    return ray;
}

void raycast_observe(Vector2 position, float angle) {
    float fidelity = 1.0f;
    int rayNumber = (int)(engine.view.dimensions.width * fidelity);
    float fov = 70 * PI / 180;
    float angleStep = fov / rayNumber;
    float angleInitial = angle - (angleStep * rayNumber / 2);

    float screenDrawStep = engine.view.dimensions.width / rayNumber;
    float wallHeight = engine.view.dimensions.height;

    for (int i = 0; i < rayNumber; i++) {
        float rayAngle = angleInitial + (i * angleStep);
        Ray2D ray = ray2D_castRay(position, rayAngle);

        // Cosine Fix
        float cosineAngle = clamp_angle(angle - rayAngle);
        ray.distance *= cos(cosineAngle);

        // Drawing Walls
        float povWallHeight = wallHeight / ray.distance;    // WALL RATIO
        if (povWallHeight > wallHeight) povWallHeight = wallHeight;

        // Textures
        Vector2 texelPosition = {
            ray.destination.x - floor(ray.destination.x),
            0
        };

        // Draw wall segment.
        for (int p = 0; p < povWallHeight; p++) {
            texelPosition.y = p / povWallHeight;
            // Color
            Color color = SAMPLE_TEX[(int)(texelPosition.x * TEX_SIZE) + (int)(texelPosition.y * TEX_SIZE) * TEX_SIZE];

            DrawRectangle(
                i * screenDrawStep,
                engine.view.dimensions.height / 2 - povWallHeight / 2 + p,
                screenDrawStep,
                1,
                color
            );
        }

        // DEBUG
        // ray2D_draw(&ray);

    }
}