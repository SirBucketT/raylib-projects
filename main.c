#include <stdio.h>
#include "raylib.h"

const float screenWidth = 1800;
const float screenHeight = 900;

float playerX;
float playerY;
float screenSpace;
float movementSpeed = 70;

bool isAlive = true;
bool bulletActive = false;

int main(void) {

    screenSpace = screenHeight - (screenHeight / 25) - 30;
    playerX = screenWidth / 2;
    playerY = screenSpace;
    float bulletX = playerX + 40;
    float bulletY = playerY -40;
    float bulletSpeedX = -10;
    float bulletSpeedY = bulletSpeedX;


    InitWindow(screenWidth, screenHeight, "useless raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const float bulletRadius = 5;
        BeginDrawing();

        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        } else if (IsKeyDown(KEY_D) && playerX + screenWidth / 20 < screenWidth) {
            playerX += movementSpeed;
        }

        if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive == true) {
            bulletX = playerX + screenWidth / 50;
            bulletY = playerY;
            bulletActive = true;
        }
        if (bulletActive) {
            DrawCircle(bulletX, bulletY - 40, bulletRadius, WHITE);
            bulletY += bulletSpeedY;
            bulletX += bulletSpeedX;

            if (bulletX - bulletRadius <= 0 || bulletX + bulletRadius >= screenWidth) {
                bulletSpeedX *= -1;
            }

            if (bulletY - bulletRadius <= 0) {
                bulletSpeedY *= -1;
            }
            if (bulletY >= screenHeight) {
                bulletActive = false;
                isAlive = false;
            }
        }

        if (CheckCollisionCircleRec((Vector2){bulletX, bulletY}, bulletRadius, (Rectangle){playerX, playerY, screenWidth / 15, screenHeight / 50})) {
            bulletSpeedY *= -1;
        }

        if (isAlive == true) {
            DrawRectangle(playerX, playerY, screenWidth / 20, screenHeight/50, WHITE);
        } else {
            DrawText("GAME OVER!", screenWidth / 2 - 100, screenHeight / 2, 50, RED);
        }

        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}