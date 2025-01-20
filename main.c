#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH 1800
#define SCREEN_HEIGHT 900
#define ROWS 8
#define COLUMNS 14
#define MAX_BLOCKS (ROWS * COLUMNS)
#define BLOCK_WIDTH 100
#define BLOCK_HEIGHT 30
#define BLOCK_SPACING 10

float playerX;
float playerY;
float movementSpeed = 50;
int OVER9000 = 9001;

typedef struct {
    float healthPoints;
    float highscore;
    float currentScore;
} ScoreData;

typedef struct {
    Rectangle rect;
    int health;
    bool active;
    Color color; 
} Block;


ScoreData player = {3, 0.0f, 0.0f};

bool isAlive = true;
bool bulletActive = false;
bool gameStarted = false;

Block blocks[MAX_BLOCKS];

float ballSpeedX;
float ballSpeedY;
const float ballSpeed = 10.0f;

void InitializeBlocks() {
    int blockIndex = 0;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            if (blockIndex >= MAX_BLOCKS) break;

            blocks[blockIndex].rect.x = col * (BLOCK_WIDTH + BLOCK_SPACING) + 100;
            blocks[blockIndex].rect.y = row * (BLOCK_HEIGHT + BLOCK_SPACING) + 50;
            blocks[blockIndex].rect.width = BLOCK_WIDTH;
            blocks[blockIndex].rect.height = BLOCK_HEIGHT;
            blocks[blockIndex].health = GetRandomValue(1, 3);
            blocks[blockIndex].active = true;

            if (blocks[blockIndex].health == 1) {
                blocks[blockIndex].color = GREEN;
            } else if (blocks[blockIndex].health == 2) {
                blocks[blockIndex].color = YELLOW;
            } else {
                blocks[blockIndex].color = RED;
            }

            blockIndex++;
        }
    }
}

void GameStarter() {
    player.healthPoints = 3;
    player.currentScore = 0;
    gameStarted = true;
    isAlive = true;
    InitializeBlocks();
}

void DrawBlocks() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, blocks[i].color);
            DrawText(TextFormat("%d", blocks[i].health),
                     blocks[i].rect.x + blocks[i].rect.width / 2 - 10,
                     blocks[i].rect.y + blocks[i].rect.height / 2 - 10,
                     20, WHITE);
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

void InitializeGame() {
    if (!gameStarted) {
        DrawText("START GAME (Y/N)", SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2, 50, WHITE);
        int key = GetKeyPressed();
        switch (key) {
            case KEY_Y:
                GameStarter();
                break;
            case KEY_N:
                CloseWindow();
                break;
            default:
                break;
        }
    }
}

void GameOver() {
    if (!isAlive && gameStarted) {
        DrawText("GAME OVER!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 50, RED);
        DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT -300, 50, WHITE);
        if (IsKeyDown(KEY_Y)) {
            GameStarter();
        } else if (IsKeyDown(KEY_N)) {
            CloseWindow();
        }
    }
}

int main(void) {
    playerX = SCREEN_WIDTH / 2;
    playerY = SCREEN_HEIGHT - 150;
    float ballX = playerX + 40;
    float ballY = playerY - 40;
    const float ballRadius = 8;
    char currentScore[50];
    char highscore[50];
    char playerLives[10];

    ballSpeedX = ballSpeed;
    ballSpeedY = -ballSpeed;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Game");
    SetTargetFPS(OVER9000);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        InitializeGame();

        sprintf(currentScore, " %.0f", player.currentScore);
        DrawText(currentScore, SCREEN_WIDTH / 2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

        if (player.currentScore > player.highscore) {
            player.highscore = player.currentScore;
        }

        sprintf(highscore, "Highscore: %.0f", player.highscore);
        DrawText(highscore, SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);
        sprintf(playerLives, "Lives: %.0f", player.healthPoints);
        DrawText(playerLives, SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);

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

        if (gameStarted == true && isAlive == true) {
            DrawRectangle(playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50, WHITE);
        }

        if (bulletActive) {
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            // BALL FRAME COLLISION:
            if (ballX - ballRadius <= 0 || ballX + ballRadius >= SCREEN_WIDTH) {
                ballSpeedX *= -1;
            }

            if (ballY - ballRadius <= 0) {
                ballSpeedY *= -1;
            }

            if (ballY + ballRadius >= SCREEN_HEIGHT) {
                bulletActive = false;
                player.healthPoints -= 1; // Game over if ball falls below screen
            }
            // PLAYER COLLISION:
            Rectangle playerRect = {playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50};
            if (CheckCollisionCircleRec((Vector2){ballX, ballY}, ballRadius, playerRect)) {
                ballSpeedY = -ballSpeed;

                float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20);
                ballSpeedX = (hitPos - 0.5f) * ballSpeed * 2.0f;
            }

            DrawCircle(ballX, ballY, ballRadius, WHITE);    // Draw ball
        }

        CheckBulletCollision(ballX, ballY, ballRadius, &player);

        DrawBlocks();

        if (player.healthPoints <= 0) {
            isAlive = false;
            GameOver();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}