#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 900

float playerX;
float playerY;
float screenSpace;
float movementSpeed = 70;

typedef struct {
    float healthPoints;
    float highscore;
    float currentScore;
} ScoreData;

typedef struct {
    int blockSize;
    int blockLine1;
}blocks;


bool isAlive = true;
bool bulletActive = false;
bool gameStarted = false;

void GameGrid() {
    blocks block = {SCREEN_WIDTH / 20, 50};
    DrawRectangle(500, block.blockLine1, block.blockSize, SCREEN_HEIGHT/50, RED);
}

void gameOverCheck() {
    if (isAlive == true && gameStarted == true) {
        DrawRectangle(playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT/50, WHITE);
    } else if (!isAlive && gameStarted == true) {
        DrawText("GAME OVER!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 50, RED);
    }
}

void InitializeGame() {
    if (!gameStarted) {
        DrawText("START GAME (Y/N)", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2, 50, WHITE);
        int key = GetKeyPressed();
        switch (key) {
            case KEY_Y:
                gameStarted = true;
            isAlive = true;
            break;
            case KEY_N:
                CloseWindow();
            break;
            default:
                break;
        }
    }
}

int main(void) {

    screenSpace = SCREEN_HEIGHT - (SCREEN_HEIGHT / 25) - 30;
    playerX = SCREEN_WIDTH / 2;
    playerY = screenSpace;
    float bulletX = playerX + 40;
    float bulletY = playerY -40;
    float bulletSpeedX = -10;
    float bulletSpeedY = bulletSpeedX;
    char highscoreText[50];


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "useless raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const float bulletRadius = 5;
        BeginDrawing();
        InitializeGame();
        ScoreData player;

        sprintf(highscoreText, "Highscore: \n %.0f", player.highscore);

        //player movement and logic
        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        } else if (IsKeyDown(KEY_D) && playerX + SCREEN_WIDTH / 20 < SCREEN_WIDTH) {
            playerX += movementSpeed;
        }

        if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive == true) {
            bulletX = playerX + SCREEN_WIDTH / 50;
            bulletY = playerY;
            bulletActive = true;
        }
        if (bulletActive) {
            DrawCircle(bulletX, bulletY - 40, bulletRadius, WHITE);
            bulletY += bulletSpeedY;
            bulletX += bulletSpeedX;

            if (bulletX - bulletRadius <= 0 || bulletX + bulletRadius >= SCREEN_WIDTH) {
                bulletSpeedX *= -1;
            }

            if (bulletY - bulletRadius <= 0) {
                bulletSpeedY *= -1;
            }
            if (bulletY >= SCREEN_HEIGHT) {
                bulletActive = false;
                isAlive = false;
            }
        }

        if (CheckCollisionCircleRec((Vector2){bulletX, bulletY}, bulletRadius, (Rectangle){playerX, playerY, SCREEN_WIDTH / 15, SCREEN_HEIGHT / 50})) {
            bulletSpeedY *= -1;
        }
        //block logic
        if (bulletActive == true && gameStarted == true && isAlive == true) {
            GameGrid();
        }

        gameOverCheck();

        DrawText(highscoreText, SCREEN_WIDTH -350, SCREEN_HEIGHT - 880, 50, WHITE);

        //score updater
        if (player.currentScore >= player.highscore) {
            player.highscore = player.currentScore;
        }

        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}