#include <stdio.h>
#include "raylib.h"

int playerX;
int playerY;
int screenSpace;
int movementSpeed = 70;

int width;
int height;

Color color;
bool isAlive = true;

int main(void) {

    const int screenWidth = 1800;
    const int screenHeight = 900;
    screenSpace = screenHeight - (screenHeight / 25) - 30;
    playerX = screenWidth / 2;
    playerY = screenSpace;


    InitWindow(screenWidth, screenHeight, "useless raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        } else if (IsKeyDown(KEY_D) && playerX + screenWidth / 25 < screenWidth) {
            playerX += movementSpeed;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawRectangle(playerX, playerY, screenWidth / 25, screenHeight/50, WHITE);
        DrawCircle(playerX + 40, playerY - 40, 5, WHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}