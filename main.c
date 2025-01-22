#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH  1800
#define SCREEN_HEIGHT 900

#define ROWS         8
#define COLUMNS      14
#define MAX_BLOCKS   (ROWS * COLUMNS)
#define BLOCK_WIDTH  100
#define BLOCK_HEIGHT 30
#define BLOCK_SPACING 10

// --------------------------------------------------------------------------------
// Global variables for game state
// --------------------------------------------------------------------------------
int destroyed     = 0;  // how many blocks have been destroyed
int upgradeValue  = 0;  // not used in this snippet, but left for expansion

float playerX;          // paddle X
float playerY;          // paddle Y
float movementSpeed = 50;

int OVER9000 = 9001;

typedef struct {
    float healthPoints;    // current # of lives
    float highscore;
    float currentScore;
} ScoreData;

typedef struct {
    Rectangle rect;
    int health;     // block health: 1, 2, or 3
    bool active;
    Color color;
} Block;

const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};

const int KONAMI_CODE_LENGTH = 10;
int playerInput[10];     // not explicitly needed, but declared
int konamiIndex = 0;

// Player data
ScoreData player = {3, 0.0f, 0.0f};

bool isAlive     = true;  // if player has >0 lives
bool gameStarted = false; // after pressing Y to start

// Single ball logic
bool  bulletActive       = false;
float ballX, ballY;
float ballSpeedX, ballSpeedY;
const float ballSpeed    = 10.0f;
const float ballRadius   = 8.0f;

// Blocks
Block blocks[MAX_BLOCKS];

// --------------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------------
void InitializeBlocks(void);
void CheckBulletCollision(float bulletX, float bulletY, float bulletRadius, ScoreData *player);
void GameStarter(void);
void InitializeGame(void);
bool IsAnyKeyPressed(void);
void Upgrades(void);
void DrawBlocks(void);
void GameOver(void);

// --------------------------------------------------------------------------------
// 1) InitializeBlocks: set block positions, random health (1-3), color based on health
// --------------------------------------------------------------------------------
void InitializeBlocks(void) {
    int blockIndex = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            if (blockIndex >= MAX_BLOCKS) break;

            blocks[blockIndex].rect.x      = col * (BLOCK_WIDTH + BLOCK_SPACING) + 100;
            blocks[blockIndex].rect.y      = row * (BLOCK_HEIGHT + BLOCK_SPACING) + 50;
            blocks[blockIndex].rect.width  = BLOCK_WIDTH;
            blocks[blockIndex].rect.height = BLOCK_HEIGHT;

            // Health: 1, 2, or 3
            blocks[blockIndex].health = GetRandomValue(1, 3);
            blocks[blockIndex].active = true;

            // Color depends on health
            if      (blocks[blockIndex].health == 1) blocks[blockIndex].color = GREEN;
            else if (blocks[blockIndex].health == 2) blocks[blockIndex].color = YELLOW;
            else                                      blocks[blockIndex].color = RED;

            blockIndex++;
        }
    }
}

// --------------------------------------------------------------------------------
// 2) CheckBulletCollision: single-ball vs. blocks collision
// --------------------------------------------------------------------------------
void CheckBulletCollision(float bulletX, float bulletY, float bulletRadius, ScoreData *player) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){bulletX, bulletY}, bulletRadius, blocks[i].rect))
        {
            // We hit this block; reduce its health
            blocks[i].health--;

            if (blocks[i].health <= 0) {
                // block destroyed
                blocks[i].active = false;
                player->currentScore += 100;
                destroyed++;
            }

            // Bounce the ball vertically
            ballSpeedY *= -1;

            // Only handle one block collision per update
            break;
        }
    }
}

// --------------------------------------------------------------------------------
// 3) GameStarter: re-initialize everything for a new game
// --------------------------------------------------------------------------------
void GameStarter(void) {
    player.healthPoints = 3;
    player.currentScore = 0;
    isAlive             = true;
    gameStarted         = true;
    destroyed           = 0;

    // Re-initialize blocks
    InitializeBlocks();

    // Place the paddle
    playerX = SCREEN_WIDTH / 2;
    playerY = SCREEN_HEIGHT - 150;

    // Automatically spawn one ball
    bulletActive = true;
    ballX        = playerX + 40;
    ballY        = playerY - 40;
    ballSpeedX   = ballSpeed;
    ballSpeedY   = -ballSpeed;
}

// --------------------------------------------------------------------------------
// 4) InitializeGame: prompt user to start or quit if not started yet
// --------------------------------------------------------------------------------
void InitializeGame(void) {
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

// --------------------------------------------------------------------------------
// 5) IsAnyKeyPressed: used for Konami code resetting
// --------------------------------------------------------------------------------
bool IsAnyKeyPressed(void) {
    for (int key = KEY_SPACE; key <= KEY_KP_9; key++) {
        if (IsKeyPressed(key)) {
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------------------
// 6) Upgrades: check for scoring thresholds, Konami code, etc.
// --------------------------------------------------------------------------------
void Upgrades(void) {
    // Example: If you exceed 1500 points, gain extra life
    if (player.currentScore > 1500) {
        player.healthPoints += 1;
    }

    // Konami code
    if (IsKeyPressed(KONAMI_CODE[konamiIndex])) {
        konamiIndex++;
        if (konamiIndex == KONAMI_CODE_LENGTH) {
            player.healthPoints = OVER9000;
            konamiIndex = 0;
        }
    }
    else if (IsAnyKeyPressed()) {
        konamiIndex = 0;
    }
}

// --------------------------------------------------------------------------------
// 7) DrawBlocks: show each block and print its health in the center
// --------------------------------------------------------------------------------
void DrawBlocks(void) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, blocks[i].color);
            DrawText(TextFormat("%d", blocks[i].health),
                     (int)(blocks[i].rect.x + blocks[i].rect.width / 2 - 10),
                     (int)(blocks[i].rect.y + blocks[i].rect.height / 2 - 10),
                     20, WHITE);
        }
    }
}

// --------------------------------------------------------------------------------
// 8) GameOver: if player has 0 lives, prompt to restart or quit
// --------------------------------------------------------------------------------
void GameOver(void) {
    if (!isAlive && gameStarted) {
        DrawText("GAME OVER!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 50, RED);
        DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT - 300, 50, WHITE);

        if (IsKeyDown(KEY_Y)) {
            GameStarter();
        }
        else if (IsKeyDown(KEY_N)) {
            CloseWindow();
        }
    }
}

// --------------------------------------------------------------------------------
// 9) main(): your main game loop
// --------------------------------------------------------------------------------
int main(void) {
    // Setup
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Single-Ball Block Game");
    SetTargetFPS(60);

    // Initial paddle position (also set again in GameStarter)
    playerX = SCREEN_WIDTH / 2;
    playerY = SCREEN_HEIGHT - 150;

    // Main loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // If the game isn't started yet, let user press Y/N
        InitializeGame();

        // Draw current score
        char currentScoreText[50];
        sprintf(currentScoreText, "%.0f", player.currentScore);
        DrawText(currentScoreText, SCREEN_WIDTH / 2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

        // Update highscore if needed
        if (player.currentScore > player.highscore) {
            player.highscore = player.currentScore;
        }

        // Check upgrades / Konami code
        Upgrades();

        // Draw highscore and lives
        DrawText(TextFormat("Highscore: %.0f", player.highscore),
                 SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);

        DrawText(TextFormat("Lives: %.0f", player.healthPoints),
                 SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);

        // Paddle movement (A for left, D for right)
        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        }
        else if (IsKeyDown(KEY_D) && playerX + SCREEN_WIDTH / 20 < SCREEN_WIDTH) {
            playerX += movementSpeed;
        }

        // Spawn a new ball if the old ball is gone, you're alive, and the game is started
        if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive && gameStarted) {
            bulletActive = true;
            ballX        = playerX + SCREEN_WIDTH / 50;  // near center of paddle
            ballY        = playerY;
            ballSpeedY   = -ballSpeed;
            ballSpeedX   = (GetRandomValue(0,1) == 0) ? -ballSpeed / 2 : ballSpeed / 2;
        }

        // Draw paddle if game is started and player is alive
        if (gameStarted && isAlive) {
            DrawRectangle((int)playerX, (int)playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50, WHITE);
        }

        // Update the single ball if active
        if (bulletActive) {
            // Move ball
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            // Left/right wall bounce
            if (ballX - ballRadius <= 0 || ballX + ballRadius >= SCREEN_WIDTH) {
                ballSpeedX *= -1;
            }
            // Top wall bounce
            if (ballY - ballRadius <= 0) {
                ballSpeedY *= -1;
            }
            // Bottom (off-screen) => lose a life, deactivate ball
            if (ballY + ballRadius >= SCREEN_HEIGHT) {
                bulletActive = false;
                player.healthPoints -= 1;
            }

            // Paddle collision
            Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50 };
            if (CheckCollisionCircleRec((Vector2){ballX, ballY}, ballRadius, playerRect)) {
                ballSpeedY = -ballSpeed;
                // Adjust horizontal speed based on where it hit the paddle
                float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20);
                ballSpeedX   = (hitPos - 0.5f) * ballSpeed * 2.0f;
            }

            // Check collisions with blocks
            CheckBulletCollision(ballX, ballY, ballRadius, &player);

            // Draw the ball
            DrawCircle((int)ballX, (int)ballY, ballRadius, WHITE);
        }

        // Draw blocks
        DrawBlocks();

        // Check if out of lives
        if (player.healthPoints <= 0) {
            isAlive = false;
            GameOver();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
