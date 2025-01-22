#include <stdio.h>
#include "raylib.h"

// --------------------------------------------------------------------------------
// Window & block definitions
// --------------------------------------------------------------------------------
#define SCREEN_WIDTH   1800
#define SCREEN_HEIGHT  900
#define ROWS           8
#define COLUMNS        14
#define MAX_BLOCKS     (ROWS * COLUMNS)
#define BLOCK_WIDTH    100
#define BLOCK_HEIGHT   30
#define BLOCK_SPACING  10

// --------------------------------------------------------------------------------
// Game-state structs
// --------------------------------------------------------------------------------
typedef struct {
    float healthPoints;    // number of lives
    float highscore;
    float currentScore;
} ScoreData;

typedef struct {
    Rectangle rect;
    int health;   // block health: 1..3
    bool active;
    Color color;
} Block;

// --------------------------------------------------------------------------------
// Global variables (unchanged names)
// --------------------------------------------------------------------------------
static ScoreData player = {3, 0.0f, 0.0f};
static Block blocks[MAX_BLOCKS];

// Paddle
static float playerX;
static float playerY;
static float movementSpeed = 50.0f;

// Single "main" ball
static bool  bulletActive = false;
static float ballX, ballY;
static float ballSpeedX, ballSpeedY;
static const float BALL_SPEED  = 10.0f;
static const float BALL_RADIUS = 8.0f;

// Control flow
static bool gameStarted = false;
static bool isAlive     = true;  // becomes false when lives run out

// Konami code
static const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};
static const int KONAMI_CODE_LENGTH = 10;
static int konamiIndex = 0;

// --------------------------------------------------------------------------------
// NEW variables for spawning 4 extra balls (without renaming existing ones)
// --------------------------------------------------------------------------------
static bool  fourBallsSpawned  = false; // track if we've already spawned them
static bool  extraBulletActive[4] = { false, false, false, false };
static float extraBallX[4];
static float extraBallY[4];
static float extraBallSpeedX[4];
static float extraBallSpeedY[4];

void GameStarter(void);
void InitializeGame(void);
void InitializeBlocks(void);
void UpdateGame(void);
void DrawGame(void);
void GameOver(void);
void CheckBallBlockCollision(void);
bool IsAnyKeyPressed(void);
void Upgrades(void);

// --------------------------------------------------------------------------------
// InitializeBlocks - sets the positions and health of all blocks
// --------------------------------------------------------------------------------
void InitializeBlocks(void) {
    int blockIndex = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            if (blockIndex >= MAX_BLOCKS) break;

            // Position
            blocks[blockIndex].rect.x = col * (BLOCK_WIDTH + BLOCK_SPACING) + 100;
            blocks[blockIndex].rect.y = row * (BLOCK_HEIGHT + BLOCK_SPACING) + 50;
            blocks[blockIndex].rect.width  = BLOCK_WIDTH;
            blocks[blockIndex].rect.height = BLOCK_HEIGHT;

            // Health: 1..3
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
// GameStarter - called whenever you press 'Y' to start or restart the game
// --------------------------------------------------------------------------------
void GameStarter(void) {
    // Reset player stats
    player.healthPoints = 3;
    player.currentScore = 0;
    isAlive             = true;
    gameStarted         = true;

    // Re-init blocks
    InitializeBlocks();

    // Reset the "spawn 4 extra balls" flag
    fourBallsSpawned = false;

    // Position paddle
    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;

    // Automatically spawn the first/main ball
    bulletActive = true;
    ballX        = playerX + 40.0f;
    ballY        = playerY - 40.0f;
    ballSpeedX   = BALL_SPEED;
    ballSpeedY   = -BALL_SPEED;

    // Reset any extra balls
    for (int i = 0; i < 4; i++) {
        extraBulletActive[i] = false;
    }
}

// --------------------------------------------------------------------------------
// InitializeGame - shows "START GAME (Y/N)" if not started
// --------------------------------------------------------------------------------
void InitializeGame(void) {
    if (!gameStarted) {
        DrawText("START GAME (Y/N)", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 50, WHITE);
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
// IsAnyKeyPressed - used for resetting Konami code
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
// Upgrades - checks Konami code, or expansions you might want
// --------------------------------------------------------------------------------
void Upgrades(void) {
    // Konami code logic
    if (IsKeyPressed(KONAMI_CODE[konamiIndex])) {
        konamiIndex++;
        if (konamiIndex == KONAMI_CODE_LENGTH) {
            player.healthPoints = 9001; // unstoppable
            konamiIndex = 0;
        }
    }
    else if (IsAnyKeyPressed()) {
        konamiIndex = 0;
    }
}

// --------------------------------------------------------------------------------
// CheckBallBlockCollision - decrements block health if hit by the main ball
// --------------------------------------------------------------------------------
void CheckBallBlockCollision(void) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, blocks[i].rect))
        {
            // Hit this block
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            // Bounce the ball vertically
            ballSpeedY *= -1.0f;
            // Only handle collision with one block per frame for the main ball
            break;
        }
    }
}

// --------------------------------------------------------------------------------
// NEW: Check collision for each of the 4 extra balls
// --------------------------------------------------------------------------------
static void CheckExtraBallsBlockCollision(int index) {
    // Index = which extra ball we're checking
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){ extraBallX[index], extraBallY[index] },
                                    BALL_RADIUS, blocks[i].rect))
        {
            // Hit this block
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            // Bounce vertically
            extraBallSpeedY[index] *= -1.0f;
            // Only one collision per frame
            break;
        }
    }
}

// --------------------------------------------------------------------------------
// NEW: Spawn 4 extra balls if score >= 4000 (only once)
// --------------------------------------------------------------------------------
static void SpawnFourBallsIfNeeded(void) {
    if (!fourBallsSpawned && player.currentScore >= 4000.0f) {
        // We'll spawn all 4 around the paddle (or near it)
        for (int i = 0; i < 4; i++) {
            extraBulletActive[i] = true;
            // Spread them out slightly so they don't all overlap
            extraBallX[i] = playerX + (i * 20);
            extraBallY[i] = playerY - 40.0f;
            extraBallSpeedX[i] = (GetRandomValue(0,1) == 0)
                                ? -BALL_SPEED / 2
                                :  BALL_SPEED / 2;
            extraBallSpeedY[i] = -BALL_SPEED;
        }
        fourBallsSpawned = true;
    }
}

// --------------------------------------------------------------------------------
// UpdateGame - handles all gameplay logic IF the player is still alive
// --------------------------------------------------------------------------------
void UpdateGame(void) {
    // 1) Paddle movement
    if (IsKeyDown(KEY_A) && playerX > 0) {
        playerX -= movementSpeed;
    }
    else if (IsKeyDown(KEY_D) && playerX + (SCREEN_WIDTH/20) < SCREEN_WIDTH) {
        playerX += movementSpeed;
    }

    // 2) If main ball is lost, press SPACE to spawn a new one (if still alive)
    if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive) {
        bulletActive = true;
        ballX = playerX + (SCREEN_WIDTH / 50);
        ballY = playerY;
        ballSpeedY = -BALL_SPEED;
        ballSpeedX = (GetRandomValue(0, 1) == 0) ? -BALL_SPEED / 2 : BALL_SPEED / 2;
    }

    // 3) Update the main ball if active
    if (bulletActive) {
        // Move
        ballX += ballSpeedX;
        ballY += ballSpeedY;

        // Collide with walls
        if (ballX - BALL_RADIUS <= 0 || ballX + BALL_RADIUS >= SCREEN_WIDTH) {
            ballSpeedX *= -1;
        }
        if (ballY - BALL_RADIUS <= 0) {
            ballSpeedY *= -1;
        }
        // Ball out of bottom => lose a life
        if (ballY + BALL_RADIUS >= SCREEN_HEIGHT) {
            bulletActive = false;
            player.healthPoints -= 1;
        }

        // Paddle collision
        Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
        if (CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, playerRect)) {
            ballSpeedY = -BALL_SPEED;
            // Adjust X speed based on where it hit the paddle
            float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20.0f);
            ballSpeedX   = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
        }

        // Block collisions (main ball)
        CheckBallBlockCollision();
    }

    // 4) Spawn 4 extra balls if we reached 4000 score
    SpawnFourBallsIfNeeded();

    // 5) Update each of the 4 extra balls if active
    for (int i = 0; i < 4; i++) {
        if (extraBulletActive[i]) {
            // Move
            extraBallX[i] += extraBallSpeedX[i];
            extraBallY[i] += extraBallSpeedY[i];

            // Collide with walls
            if (extraBallX[i] - BALL_RADIUS <= 0 || extraBallX[i] + BALL_RADIUS >= SCREEN_WIDTH) {
                extraBallSpeedX[i] *= -1.0f;
            }
            if (extraBallY[i] - BALL_RADIUS <= 0) {
                extraBallSpeedY[i] *= -1.0f;
            }
            // Ball out of bottom => lose a life
            if (extraBallY[i] + BALL_RADIUS >= SCREEN_HEIGHT) {
                extraBulletActive[i] = false;
                player.healthPoints -= 1;
            }

            // Paddle collision
            Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
            if (CheckCollisionCircleRec((Vector2){ extraBallX[i], extraBallY[i] },
                                        BALL_RADIUS, playerRect))
            {
                extraBallSpeedY[i] = -BALL_SPEED;
                float hitPos = (extraBallX[i] - playerX) / (SCREEN_WIDTH / 20.0f);
                extraBallSpeedX[i] = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
            }

            // Block collisions (extra ball i)
            CheckExtraBallsBlockCollision(i);
        }
    }

    // 6) If health is <= 0, it's game over
    if (player.healthPoints <= 0) {
        player.healthPoints = 0;
        isAlive             = false;
        bulletActive        = false; // no main ball
        // Deactivate all extra balls
        for (int i = 0; i < 4; i++) {
            extraBulletActive[i] = false;
        }
    }
}

// --------------------------------------------------------------------------------
// DrawGame - draws all game elements IF the player is alive
// --------------------------------------------------------------------------------
void DrawGame(void) {
    // Draw the score
    DrawText(TextFormat("%.0f", player.currentScore),
             SCREEN_WIDTH/2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

    // Update and draw highscore
    if (player.currentScore > player.highscore) {
        player.highscore = player.currentScore;
    }
    DrawText(TextFormat("Highscore: %.0f", player.highscore),
             SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);

    // Draw lives
    DrawText(TextFormat("Lives: %.0f", player.healthPoints),
             SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);

    // Draw paddle
    DrawRectangle((int)playerX, (int)playerY, SCREEN_WIDTH/20, SCREEN_HEIGHT/50, WHITE);

    // Draw the main ball if active
    if (bulletActive) {
        DrawCircle((int)ballX, (int)ballY, BALL_RADIUS, WHITE);
    }

    // Draw the 4 extra balls if active
    for (int i = 0; i < 4; i++) {
        if (extraBulletActive[i]) {
            DrawCircle((int)extraBallX[i], (int)extraBallY[i], BALL_RADIUS, WHITE);
        }
    }

    // Draw blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, blocks[i].color);
            // Display block health
            DrawText(TextFormat("%d", blocks[i].health),
                     (int)(blocks[i].rect.x + blocks[i].rect.width/2 - 10),
                     (int)(blocks[i].rect.y + blocks[i].rect.height/2 - 10),
                     20, WHITE);
        }
    }
}

void GameOver(void) {
    DrawText("GAME OVER!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 50, RED);
    DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 + 60, 50, WHITE);

    if (IsKeyPressed(KEY_Y)) {
        GameStarter();
    }
    else if (IsKeyPressed(KEY_N)) {
        CloseWindow();
    }
}

int main(void) {
    // Setup
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block kuzushi raylib game build");
    SetTargetFPS(60);

    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        if (!gameStarted) {
            InitializeGame();
        }
        else if (!isAlive) {
            GameOver();
        }
        else {
            Upgrades();
            UpdateGame();
            DrawGame();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}