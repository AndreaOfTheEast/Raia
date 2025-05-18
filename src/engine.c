#include "../includes/engine.h"
#include "../includes/objects.h"
#include "../includes/raymath.h"
#include "../includes/utils.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Engine engine;

void engine_init() {
    engine.window.dimensions = (Rectangle) {
        0, 0, 640, 640
    };
    engine.targetFPS = 60;

    InitWindow(
        engine.window.dimensions.width, 
        engine.window.dimensions.height, 
        "Raia"
    );
    SetTargetFPS(engine.targetFPS);
    //ToggleFullscreen();

    engine.view.dimensions = (Rectangle) {
        0, 0, 
        engine.window.dimensions.width / 2,
        engine.window.dimensions.height / 2
    };
    engine.view.renderTexture = LoadRenderTexture(
        engine.view.dimensions.width,
        engine.view.dimensions.height
    );

    // Load Sprites
    engine.spriteAtlas = LoadImage("assets/SpriteAtlas.png");
    engine.spriteAmount = engine.spriteAtlas.width * engine.spriteAtlas.height / TEX_SIZE;

    engine.sprites = malloc(sizeof(Sprite) * engine.spriteAmount);

    for (int i = 0; i < engine.spriteAmount; i++) {
        engine.sprites[i].sprite = ImageFromImage(engine.spriteAtlas, (Rectangle) {
            (i % TEX_SIZE) * TEX_SIZE,
            0,
            TEX_SIZE,
            TEX_SIZE
        });
        engine.sprites[i].pixels = LoadImageColors(engine.sprites[i].sprite);
    }

    // Handle Object Initialization
    objects_init();

}

void engine_deinit() {
    for (int i = 0; i < engine.spriteAmount; i++) {
        UnloadImage(engine.sprites[i].sprite);
        UnloadImageColors(engine.sprites[i].pixels);
    }
    free(engine.sprites);
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
    DrawFPS(8, 8);
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
    float fov = 60 * PI / 180;
    float angleStep = fov / rayNumber;
    float angleInitial = angle - (angleStep * rayNumber / 2);

    float screenDrawStep = engine.view.dimensions.width / rayNumber;
    float wallHeight = engine.view.dimensions.height;

    for (int i = 0; i < rayNumber; i++) {
        float rayAngle = angleInitial + (i * angleStep);
        Ray2D ray = ray2D_castRay(position, rayAngle);

        // Drawing Walls
        float overcorrectionCoef = 1.0f;
        float fixedAngle = cos(clamp_angle((angle - rayAngle) * overcorrectionCoef)); // Get the orthographic angle to correct for fish eye.

        ray.distance *= fixedAngle;         // Cosine Fix

        float povWallHeight = wallHeight / ray.distance;    // WALL RATIO

        float texelYStep = 1 / povWallHeight;
        float texelYOffset = 0;
        float garbageShitFix = (engine.view.dimensions.height / 320) * 158;

        // Clamping walls to not draw bigger than the screen.
        if (povWallHeight > wallHeight) {
            texelYOffset = povWallHeight - wallHeight; // When we chop off the excess we need to save the texture offset to begin the texture strip in the right place.
            povWallHeight = wallHeight;
        }

        // Build Screen Cross Section
        Vector2 texelPosition = { 0.0f, 0.0f };
        int wallStart = engine.view.dimensions.height / 2 - povWallHeight / 2; // Represents the top of the wall segment.
        int p = 0; // Pixel iterator for wall and floor segment.


        // Draw ceiling segment.
        for (int j = wallStart; j >= 0; j--) {
            float pDeltaY = j - (engine.view.dimensions.height / 2.0);    // Distance of projection plane pixel from the horizon.

            // Texture
            texelPosition = (Vector2) { 
                objects.player.position.x - cos(rayAngle) * garbageShitFix / pDeltaY / fixedAngle, 
                objects.player.position.y - sin(rayAngle) * garbageShitFix / pDeltaY / fixedAngle
            };

            texelPosition.x = texelPosition.x - floor(texelPosition.x);
            texelPosition.y = texelPosition.y - floor(texelPosition.y);

            DrawRectangle(
                i * screenDrawStep,
                j,
                screenDrawStep,
                1,
                engine.sprites[2].pixels[(int)(texelPosition.x * TEX_SIZE) + (int)(texelPosition.y * TEX_SIZE) * TEX_SIZE]
            );
        }

        // Draw wall segment.
        // Set Texture Position
        if (ray.direction == 0) {
            texelPosition.x = ray.destination.x - floor(ray.destination.x);
        } else {
            texelPosition.x = ray.destination.y - floor(ray.destination.y);
        }
        texelPosition.y = texelYStep * texelYOffset / 2;

        for (p = wallStart; p < wallStart + povWallHeight; p++) {

            // Texture
            Color color = engine.sprites[1].pixels[(int)(texelPosition.x * TEX_SIZE) + (int)(texelPosition.y * TEX_SIZE) * TEX_SIZE];
            if (ray.direction == 1) color = ColorBrightness(color, -0.3);

            DrawRectangle(
                i * screenDrawStep,
                p,
                screenDrawStep,
                1,
                color
            );

            texelPosition.y += texelYStep;
        }

        // Draw floor segment.
        for (;p < engine.view.dimensions.height; p++) {
            float pDeltaY = p - (engine.view.dimensions.height / 2.0);    // Distance of projection plane pixel from the horizon.

            texelPosition = (Vector2) { 
                objects.player.position.x + cos(rayAngle) * garbageShitFix / pDeltaY / fixedAngle, 
                objects.player.position.y + sin(rayAngle) * garbageShitFix / pDeltaY / fixedAngle
            };

            texelPosition.x = texelPosition.x - floor(texelPosition.x);
            texelPosition.y = texelPosition.y - floor(texelPosition.y);

            DrawRectangle(
                i * screenDrawStep,
                p,
                screenDrawStep,
                1,
                engine.sprites[0].pixels[(int)(texelPosition.x * TEX_SIZE) + (int)(texelPosition.y * TEX_SIZE) * TEX_SIZE]
            );
        }

        // DEBUG
        // ray2D_draw(&ray);

    }
}