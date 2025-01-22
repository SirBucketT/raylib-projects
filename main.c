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
// Global variables
// --------------------------------------------------------------------------------
static ScoreData player = {3, 0.0f, 0.0f};
static Block blocks[MAX_BLOCKS];

// Paddle
static float playerX;
static float playerY;
static float movementSpeed = 50.0f;

// Ball
static bool  bulletActive = false;
static float ballX, ballY;
static float ballSpeedX, ballSpeedY;
static const float BALL_SPEED  = 10.0f;
static const float BALL_RADIUS = 8.0f;

// Control flow
static bool gameStarted = false;
static bool isAlive     = true;  // becomes false when lives run out

// Konami code (optional cheat code)
static const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};
static const int KONAMI_CODE_LENGTH = 10;
static int konamiIndex = 0;

// --------------------------------------------------------------------------------
// Forward declarations
// --------------------------------------------------------------------------------
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

    // Position paddle
    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;

    // Automatically spawn the first ball
    bulletActive = true;
    ballX        = playerX + 40.0f;
    ballY        = playerY - 40.0f;
    ballSpeedX   = BALL_SPEED;
    ballSpeedY   = -BALL_SPEED;
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
// Upgrades - checks Konami code, or any expansions you might want
// --------------------------------------------------------------------------------
void Upgrades(void) {
    // Remove or comment out the old "score > 1500 => +1 life" to avoid extra lives
    // if (player.currentScore > 1500) {
    //     player.healthPoints += 1; // removed to avoid unwanted free lives
    // }

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
// CheckBallBlockCollision - decrements block health if hit
// --------------------------------------------------------------------------------
void CheckBallBlockCollision(void) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, blocks[i].rect)) {

            // Hit this block
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            // Bounce the ball vertically
            ballSpeedY *= -1.0f;
            // Only handle collision with one block per frame
            break;
        }
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

    // 2) If ball is lost, press SPACE to spawn a new one (if still alive)
    if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive) {
        bulletActive = true;
        ballX = playerX + (SCREEN_WIDTH / 50);
        ballY = playerY;
        ballSpeedY = -BALL_SPEED;
        ballSpeedX = (GetRandomValue(0, 1) == 0) ? -BALL_SPEED / 2 : BALL_SPEED / 2;
    }

    // 3) Update the ball if active
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

        // Block collisions
        CheckBallBlockCollision();
    }

    // 4) If health is <= 0, it's game over
    if (player.healthPoints <= 0) {
        player.healthPoints = 0;
        isAlive             = false;
        bulletActive        = false; // no ball should remain active
    }
}

// --------------------------------------------------------------------------------
// DrawGame - draws all game elements IF the player is alive
// --------------------------------------------------------------------------------
void DrawGame(void) {
    // Draw the score
    DrawText(TextFormat("%.0f", player.currentScore), SCREEN_WIDTH/2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

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

    // Draw ball if active
    if (bulletActive) {
        DrawCircle((int)ballX, (int)ballY, BALL_RADIUS, WHITE);
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

// --------------------------------------------------------------------------------
// GameOver - when isAlive == false
// --------------------------------------------------------------------------------
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

// --------------------------------------------------------------------------------
// main() - the game loop
// --------------------------------------------------------------------------------
int main(void) {
    // Setup
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block kuzushi raylib game build");
    SetTargetFPS(60);

    // Initial positions (overridden in GameStarter anyway)
    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;

    // Main loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        // 1) If not started, show start prompt
        if (!gameStarted) {
            InitializeGame();
        }
        // 2) Else if we're started but out of lives, show Game Over
        else if (!isAlive) {
            GameOver();
        }
        // 3) Else do normal gameplay
        else {
            // Upgrades (Konami code) first
            Upgrades();
            // Update logic
            UpdateGame();
            // Then draw everything
            DrawGame();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
