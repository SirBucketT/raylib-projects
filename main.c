#include <stdio.h>
#include "raylib.h"

int circleX = 900;
int circleY = 450;
int movementSpeed = 20;

int main(void) {

    const int screenWidth = 1800;
    const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Raylib bullshit window");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if (IsKeyDown(KEY_A)) {
            circleX -= movementSpeed;
        } else if (IsKeyDown(KEY_D)) {
            circleX += movementSpeed;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(circleX, circleY, 20, GRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}