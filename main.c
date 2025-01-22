#include <stdio.h>
#include "raylib.h"

// --------------------------------------------------------------------------------
// Screen and game constants
// --------------------------------------------------------------------------------
#define SCREEN_WIDTH  1800
#define SCREEN_HEIGHT 900

#define ROWS 8
#define COLUMNS 14
#define MAX_BLOCKS (ROWS * COLUMNS)
#define BLOCK_WIDTH  100
#define BLOCK_HEIGHT 30
#define BLOCK_SPACING 10

#define OVER9000 9001

// --------------------------------------------------------------------------------
// Types and global variables
// --------------------------------------------------------------------------------
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

// For multi-ball
typedef struct {
    float x, y;
    float speedX, speedY;
    float radius;
    bool active;
} Ball;

#define MAX_BALLS 10         // max number of simultaneous balls
static Ball balls[MAX_BALLS];

static Block blocks[MAX_BLOCKS];

static ScoreData player = {3, 0.0f, 0.0f};

// Player/paddle related
static float playerX;
static float playerY;
static float movementSpeed = 50;
static bool isAlive = true;
static bool gameStarted = false;

// Keep track of how many blocks have been destroyed
static int destroyed = 0;
static int upgradeValue = 0;  // (Left unchanged, use as needed)

// For multi-ball, base speed
static const float ballSpeed = 10.0f;

// For the 4-ball spawn
static bool fourBallsSpawned = false;

// Konami code
static const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};
static const int KONAMI_CODE_LENGTH = 10;
static int playerInput[10];
static int konamiIndex = 0;

// --------------------------------------------------------------------------------
// Forward declarations
// --------------------------------------------------------------------------------
void InitializeBlocks(void);
void InitializeBalls(void);
void GameStarter(void);
void InitializeGame(void);
void DrawBlocks(void);
void CheckBulletCollision(float bulletX, float bulletY, float bulletRadius, ScoreData *player);
bool IsAnyKeyPressed(void);
void Upgrades(void);
void GameOver(void);
void SpawnFourBalls(void);

// --------------------------------------------------------------------------------
// Function definitions
// --------------------------------------------------------------------------------

// Initialize all block data
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

// Initialize all balls to inactive
void InitializeBalls(void) {
    for (int i = 0; i < MAX_BALLS; i++) {
        balls[i].x = 0;
        balls[i].y = 0;
        balls[i].speedX = 0;
        balls[i].speedY = 0;
        balls[i].radius = 8.0f;
        balls[i].active = false;
    }
}

// Start or restart the game
void GameStarter() {
    player.healthPoints = 3;
    player.currentScore = 0;
    destroyed = 0;
    gameStarted = true;
    isAlive = true;

    // Re-initialize blocks and balls
    InitializeBlocks();
    InitializeBalls();

    // Reset the "4-ball spawn" so it can happen again
    fourBallsSpawned = false;
}

void DrawBlocks() {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, blocks[i].color);
            // Draw health in the center of the block
            DrawText(TextFormat("%d", blocks[i].health),
                     (int)(blocks[i].rect.x + blocks[i].rect.width / 2 - 10),
                     (int)(blocks[i].rect.y + blocks[i].rect.height / 2 - 10),
                     20, WHITE);
        }
    }
}

// Checks collision of a "ball" with all blocks
void CheckBulletCollision(float bulletX, float bulletY, float bulletRadius, ScoreData *player) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){bulletX, bulletY}, bulletRadius, blocks[i].rect)) {

            // If collision, reduce health and bounce
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player->currentScore += 100;
                destroyed++;
            }
            // Reverse vertical direction after hitting a block
            // (The caller will handle the actual ball's speedY)
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

bool IsAnyKeyPressed() {
    for (int key = KEY_SPACE; key <= KEY_KP_9; key++) {
        if (IsKeyPressed(key)) {
            return true;
        }
    }
    return false;
}

void Upgrades() {
    if (player.currentScore > 1500) {
        player.healthPoints += 1;
    }
    // Konami code logic
    if (IsKeyPressed(KONAMI_CODE[konamiIndex])) {
        konamiIndex++;
        if (konamiIndex == KONAMI_CODE_LENGTH) {
            player.healthPoints = OVER9000;
            konamiIndex = 0;
        }
    } else if (IsAnyKeyPressed()) {
        konamiIndex = 0;
    }
}

// Spawns 4 balls near the player's paddle (only once)
void SpawnFourBalls(void) {
    if (!fourBallsSpawned) {
        for (int i = 0; i < 4; i++) {
            // Find an inactive ball slot
            for (int j = 0; j < MAX_BALLS; j++) {
                if (!balls[j].active) {
                    balls[j].x      = playerX + SCREEN_WIDTH / 50;  // near the paddle
                    balls[j].y      = playerY;
                    balls[j].speedY = -ballSpeed;
                    // Random X velocity so they spread out
                    balls[j].speedX = (float)GetRandomValue(-5, 5);
                    balls[j].radius = 8.0f;
                    balls[j].active = true;
                    break;  // move to next i
                }
            }
        }
        fourBallsSpawned = true; // ensure this happens only once
    }
}

void GameOver() {
    if (!isAlive && gameStarted) {
        DrawText("GAME OVER!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 50, RED);
        DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT - 300, 50, WHITE);
        if (IsKeyDown(KEY_Y)) {
            GameStarter();
        } else if (IsKeyDown(KEY_N)) {
            CloseWindow();
        }
    }
}

// --------------------------------------------------------------------------------
// Main
// --------------------------------------------------------------------------------
int main(void) {
    // Initial paddle position
    playerX = SCREEN_WIDTH / 2;
    playerY = SCREEN_HEIGHT - 150;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Multi-Ball Block Game");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // If game not started, show the "Start Game" prompt
        InitializeGame();

        // Draw current score
        char currentScoreText[50];
        sprintf(currentScoreText, " %.0f", player.currentScore);
        DrawText(currentScoreText, SCREEN_WIDTH / 2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

        // Update highscore
        if (player.currentScore > player.highscore) {
            player.highscore = player.currentScore;
        }

        // Check and apply upgrades (konami, extra life, etc.)
        Upgrades();

        // Draw highscore and player lives
        char highscoreText[50];
        sprintf(highscoreText, "Highscore: %.0f", player.highscore);
        DrawText(highscoreText, SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);

        char playerLivesText[20];
        sprintf(playerLivesText, "Lives: %.0f", player.healthPoints);
        DrawText(playerLivesText, SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);

        // Simple left-right movement for the paddle
        if (IsKeyDown(KEY_A) && playerX > 0) {
            playerX -= movementSpeed;
        }
        else if (IsKeyDown(KEY_D) && playerX + SCREEN_WIDTH / 20 < SCREEN_WIDTH) {
            playerX += movementSpeed;
        }

        // Press SPACE to spawn a ball (if there's a free slot)
        if (IsKeyPressed(KEY_SPACE) && isAlive) {
            // Find an inactive ball slot
            for (int i = 0; i < MAX_BALLS; i++) {
                if (!balls[i].active) {
                    // Spawn a new ball
                    balls[i].x      = playerX + SCREEN_WIDTH / 50;
                    balls[i].y      = playerY;
                    balls[i].speedY = -ballSpeed;
                    // Randomly shift left or right
                    balls[i].speedX = (GetRandomValue(0, 1) == 0 ? -ballSpeed / 2 : ballSpeed / 2);
                    balls[i].radius = 8.0f;
                    balls[i].active = true;
                    break;
                }
            }
        }

        // Draw the paddle if the game is started and player is alive
        if (gameStarted && isAlive) {
            DrawRectangle((int)playerX, (int)playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50, WHITE);
        }

        // --- Spawn 4 extra balls at score >= 2000 (only once) ---
        if (player.currentScore >= 2000) {
            SpawnFourBalls();
        }

        // Update each active ball
        for (int i = 0; i < MAX_BALLS; i++) {
            if (!balls[i].active) continue;

            // Move the ball
            balls[i].x += balls[i].speedX;
            balls[i].y += balls[i].speedY;

            // Check collisions with screen edges
            if (balls[i].x - balls[i].radius <= 0 || balls[i].x + balls[i].radius >= SCREEN_WIDTH) {
                balls[i].speedX *= -1;
            }
            if (balls[i].y - balls[i].radius <= 0) {
                balls[i].speedY *= -1;
            }
            if (balls[i].y + balls[i].radius >= SCREEN_HEIGHT) {
                // Ball falls off bottom
                balls[i].active = false;
                player.healthPoints -= 1;
            }

            // Player paddle collision
            Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50 };
            if (CheckCollisionCircleRec((Vector2){balls[i].x, balls[i].y}, balls[i].radius, playerRect)) {
                balls[i].speedY = -ballSpeed;

                float hitPos = (balls[i].x - playerX) / (SCREEN_WIDTH / 20);
                balls[i].speedX = (hitPos - 0.5f) * ballSpeed * 2.0f;
            }

            // Check collision with blocks
            // (If a collision occurs, CheckBulletCollision bounces vertical speed, so handle that)
            float oldSpeedY = balls[i].speedY;
            CheckBulletCollision(balls[i].x, balls[i].y, balls[i].radius, &player);
            if (oldSpeedY != balls[i].speedY) {
                // If you want to bounce the ball upon block collision, you can do:
                balls[i].speedY *= -1.0f;
            }

            // Draw the ball
            DrawCircle((int)balls[i].x, (int)balls[i].y, balls[i].radius, WHITE);
        }

        // Draw the blocks
        DrawBlocks();

        // Check for game over
        if (player.healthPoints <= 0) {
            isAlive = false;
            GameOver();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
