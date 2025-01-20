#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 900
#define MAX_BLOCKS 10

float playerX;
float playerY;
float movementSpeed = 70;

typedef struct {
    float healthPoints;
    float highscore;
    float currentScore;
} ScoreData;

typedef struct {
    Rectangle rect;
    int health;
    bool active;
} Block;

bool isAlive = true;
bool bulletActive = false;
bool gameStarted = false;

Block blocks[MAX_BLOCKS];

float ballSpeedX;
float ballSpeedY;
const float ballSpeed = 10.0f;

void InitializeBlocks() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        blocks[i].rect.x = 200 + i * 120;
        blocks[i].rect.y = 100;
        blocks[i].rect.width = SCREEN_WIDTH / 20;
        blocks[i].rect.height = SCREEN_HEIGHT / 50;
        blocks[i].health = 3;
        blocks[i].active = true;
    }
}

void DrawBlocks() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, RED);
            DrawText(TextFormat("%d", blocks[i].health), blocks[i].rect.x + 20, blocks[i].rect.y + 10, 20, WHITE);
        }
    }
}

void CheckBulletCollision(float bulletX, float bulletY, float bulletRadius, ScoreData *player) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){bulletX, bulletY}, bulletRadius, blocks[i].rect)) {
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player->currentScore += 100;
            }
            ballSpeedY *= -1; // Bounce the ball vertically
            break;
        }
    }
}

void gameOverCheck() {
    if (!isAlive && gameStarted) {
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
                InitializeBlocks();
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
    playerX = SCREEN_WIDTH / 2;
    playerY = SCREEN_HEIGHT - 50;
    float ballX = playerX + 40;
    float ballY = playerY - 40;
    const float ballRadius = 5;
    char highscoreText[50];
    char playerLives[1];

    ballSpeedX = ballSpeed;
    ballSpeedY = -ballSpeed;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raylib Block Game");
    SetTargetFPS(60);

    ScoreData player = {3, 0.0f, 0.0f};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (player.currentScore >= player.highscore) {
            player.highscore = player.currentScore;
        }

        InitializeGame();

        sprintf(playerLives, "Lives: %i", playerLives);
        if (player.healthPoints == 3) {
            DrawText(playerLives, SCREEN_WIDTH, SCREEN_HEIGHT, 50, WHITE);
        }
        sprintf(highscoreText, "Highscore: %.0f", player.highscore);
        DrawText(highscoreText, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 880, 50, WHITE);

        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        } else if (IsKeyDown(KEY_D) && playerX + SCREEN_WIDTH / 20 < SCREEN_WIDTH) {
            playerX += movementSpeed;
        }

        if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive) {
            bulletActive = true;
            ballX = playerX + SCREEN_WIDTH / 50;
            ballY = playerY;

            ballSpeedY = -ballSpeed;

            ballSpeedX = GetRandomValue(0, 1) == 0 ? -ballSpeed / 2 : ballSpeed / 2;
        }

        if (gameStarted == true, isAlive == true) {
            DrawRectangle(playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50, WHITE);
        }

        if (bulletActive) {
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            if (ballX - ballRadius <= 0 || ballX + ballRadius >= SCREEN_WIDTH) {
                ballSpeedX *= -1;
            }

            if (ballY - ballRadius <= 0) {
                ballSpeedY *= -1;
            }

            Rectangle playerRect = {playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50};
            if (CheckCollisionCircleRec((Vector2){ballX, ballY}, ballRadius, playerRect)) {
                ballSpeedY = -ballSpeed; // Reverse vertical direction (upward)

                // Adjust horizontal speed based on where the ball hits the paddle
                float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20);
                ballSpeedX = (hitPos - 0.5f) * ballSpeed * 2.0f; // Map hitPos (0.0-1.0) to a range
            }

            if (ballY + ballRadius >= SCREEN_HEIGHT) {
                bulletActive = false;
                player.healthPoints -= 1; // Game over if ball falls below screen
            }

            DrawCircle(ballX, ballY, ballRadius, WHITE);
        }
        if (player.healthPoints <= 0) {
            isAlive = false;
        }

        CheckBulletCollision(ballX, ballY, ballRadius, &player);

        DrawBlocks();

        gameOverCheck();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
