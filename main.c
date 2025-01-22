#include <math.h>
#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH   1800
#define SCREEN_HEIGHT  900
#define ROWS           14
#define COLUMNS        14
#define MAX_BLOCKS     (ROWS * COLUMNS)
#define BLOCK_WIDTH    100
#define BLOCK_HEIGHT   30
#define BLOCK_SPACING  10

typedef struct Block {
    Rectangle rect;
    int health;
    bool active;
    Color color;
} Block;

typedef struct {
    int currentRows;
    int currentCols;
} blocksRow;

typedef struct {
    float HP;
    float highscore;
    float currentScore;
} playerDataManager;

typedef enum {
    NOT_STARTED,
    GAME_OVER,
    GAME_WIN,
    GAME_PLAYING
} GameFlowState;

GameFlowState currentState;

// Global variables
blocksRow level             = {2, 14};
playerDataManager player    = {10, 0.0f, 0.0f};
Block blocks[MAX_BLOCKS];

// Paddle
float playerX;
float playerY;
float movementSpeed = 50.0f;

// Single main ball
bool  ball_active = false;
float ballX, ballY;
float ballSpeedX, ballSpeedY;
const float BALL_SPEED  = 10.0f;
const float BALL_RADIUS = 8.0f;

// Control flow booleans
bool gameStarted = false;
bool isAlive     = true;
bool gameWon     = false;

// Konami code
const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};
const int KONAMI_CODE_LENGTH = 10;
int konamiIndex = 0;

// Extra balls
bool  fourBallsSpawned    = false;
bool  extraBallsActive[4] = { false, false, false, false };
float extraBallX[4];
float extraBallY[4];
float extraBallSpeedX[4];
float extraBallSpeedY[4];

// ----------------------------------------------------------------------
// Forward declarations
// ----------------------------------------------------------------------
void GameStarter(void);
void InitializeGame(void);
void InitializeBlocks(void);
void UpdateGame(void);
void DrawGame(void);
void GameOver(void);
void WinScreen(void);
bool IsAnyKeyPressed(void);
void Upgrades(void);
void levelReset(void);
void dataLoader(bool load);
bool AllBlocksCleared(void);
void SpawnFourBallsIfNeeded(void);
void GameState(void);
void CheckBlockCollision(float *posX, float *posY, float radius, float *speedX, float *speedY);

// ----------------------------------------------------------------------
//  Determine the current game state based on booleans
// ----------------------------------------------------------------------
void GameState(void) {
    if (!gameStarted) {
        currentState = NOT_STARTED;
    }
    else if (!isAlive && !gameWon) {
        currentState = GAME_OVER;
    }
    else if (gameWon) {
        currentState = GAME_WIN;
    }
    else {
        currentState = GAME_PLAYING;
    }
}

// ----------------------------------------------------------------------
//  Unified collision check for main + extra balls
// ----------------------------------------------------------------------
void CheckBlockCollision(float *posX, float *posY, float radius, float *speedX, float *speedY) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){*posX, *posY}, radius, blocks[i].rect)) {

            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            // Reverse only the Y speed
            *speedY *= -1.0f;
            break;
        }
    }
}

// ----------------------------------------------------------------------
//  Checks if all blocks are cleared
// ----------------------------------------------------------------------
bool AllBlocksCleared(void) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) return false;
    }
    return true;
}

// ----------------------------------------------------------------------
//  Sets up block positions, health, and colors
// ----------------------------------------------------------------------
void InitializeBlocks(void) {
    int blockIndex = 0;
    for (int row = 0; row < level.currentRows; row++) {
        for (int col = 0; col < level.currentCols; col++) {
            if (blockIndex >= MAX_BLOCKS) break;
            blocks[blockIndex].rect.x = col * (BLOCK_WIDTH + BLOCK_SPACING) + 100;
            blocks[blockIndex].rect.y = row * (BLOCK_HEIGHT + BLOCK_SPACING) + 50;
            blocks[blockIndex].rect.width  = BLOCK_WIDTH;
            blocks[blockIndex].rect.height = BLOCK_HEIGHT;

            blocks[blockIndex].health = GetRandomValue(1, 3);
            blocks[blockIndex].active = true;

            if      (blocks[blockIndex].health == 1) blocks[blockIndex].color = GREEN;
            else if (blocks[blockIndex].health == 2) blocks[blockIndex].color = YELLOW;
            else                                      blocks[blockIndex].color = RED;

            blockIndex++;
        }
    }
}

// ----------------------------------------------------------------------
//  Initializes a new game round
// ----------------------------------------------------------------------
void GameStarter(void) {
    player.currentScore = 0;
    isAlive             = true;
    gameStarted         = true;
    gameWon             = false;
    InitializeBlocks();
    fourBallsSpawned    = false;

    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;
    ball_active = true;
    ballX      = playerX + 40.0f;
    ballY      = playerY - 40.0f;
    ballSpeedX = BALL_SPEED;
    ballSpeedY = -BALL_SPEED;

    // deactivate extra bullets
    for (int i = 0; i < 4; i++) extraBallsActive[i] = false;
}

// ----------------------------------------------------------------------
//  Displays and waits for user input to start or quit
// ----------------------------------------------------------------------
void InitializeGame(void) {
    DrawText("START GAME (Y/N)", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 50, WHITE);
    int key = GetKeyPressed();
    switch (key) {
        case KEY_Y:
            GameStarter();
            break;
        case KEY_N:
            CloseWindow();
            dataLoader(false);
            break;
        default:
            break;
    }
}

// ----------------------------------------------------------------------
//  Check for *any* key pressed (for Konami code logic)
// ----------------------------------------------------------------------
bool IsAnyKeyPressed(void) {
    for (int key = KEY_SPACE; key <= KEY_KP_9; key++) {
        if (IsKeyPressed(key)) return true;
    }
    return false;
}

// ----------------------------------------------------------------------
//  Checks Konami code input
// ----------------------------------------------------------------------
void Upgrades(void) {
    if (IsKeyPressed(KONAMI_CODE[konamiIndex])) {
        konamiIndex++;
        if (konamiIndex == KONAMI_CODE_LENGTH) {
            player.HP += 9001;
            konamiIndex = 0;
        }
    }
    else if (IsAnyKeyPressed()) {
        konamiIndex = 0;
    }
}

// ----------------------------------------------------------------------
//  Spawns 4 additional balls if player's score >= 4000
// ----------------------------------------------------------------------
void SpawnFourBallsIfNeeded(void) {
    if (!fourBallsSpawned && player.currentScore >= 4000.0f) {
        for (int i = 0; i < 4; i++) {
            extraBallsActive[i] = true;
            extraBallX[i]        = ballX;
            extraBallY[i]        = ballY;

            float angle = GetRandomValue(0, 359) * DEG2RAD;
            extraBallSpeedX[i] = cosf(angle) * BALL_SPEED;
            extraBallSpeedY[i] = sinf(angle) * BALL_SPEED;
        }
        fourBallsSpawned = true;
    }
}

// ----------------------------------------------------------------------
//  Main gameplay logic
// ----------------------------------------------------------------------
void UpdateGame(void) {
    // Launch the ball if space is pressed and ball is not active
    if (IsKeyPressed(KEY_SPACE) && !ball_active && isAlive) {
        ball_active = true;
        ballX       = playerX + (SCREEN_WIDTH / 50);
        ballY       = playerY;
        ballSpeedY  = -BALL_SPEED;
        ballSpeedX  = (GetRandomValue(0, 1) == 0) ? -BALL_SPEED / 2 : BALL_SPEED / 2;
    }

    // Main ball
    if (ball_active) {
        ballX += ballSpeedX;
        ballY += ballSpeedY;

        // Bounce off left/right walls
        if (ballX - BALL_RADIUS <= 0 || ballX + BALL_RADIUS >= SCREEN_WIDTH) {
            ballSpeedX *= -1;
        }
        // Bounce off top
        if (ballY - BALL_RADIUS <= 0) {
            ballSpeedY *= -1;
        }
        // If ball hits bottom, lose a life
        if (ballY + BALL_RADIUS >= SCREEN_HEIGHT) {
            ball_active = false;
            player.HP -= 1;
        }

        // Paddle collision
        Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
        if (CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, playerRect)) {
            ballSpeedY = -BALL_SPEED;
            float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20.0f);
            ballSpeedX   = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
        }
        CheckBlockCollision(&ballX, &ballY, BALL_RADIUS, &ballSpeedX, &ballSpeedY);
    }

    SpawnFourBallsIfNeeded();

    //logic for extra balls
    for (int i = 0; i < 4; i++) {
        if (extraBallsActive[i]) {
            extraBallX[i] += extraBallSpeedX[i];
            extraBallY[i] += extraBallSpeedY[i];

            // bounce off left/right walls
            if (extraBallX[i] - BALL_RADIUS <= 0 || extraBallX[i] + BALL_RADIUS >= SCREEN_WIDTH) {
                extraBallSpeedX[i] *= -1.0f;
            }
            // bounce off top
            if (extraBallY[i] - BALL_RADIUS <= 0) {
                extraBallSpeedY[i] *= -1.0f;
            }
            // hits bottom
            if (extraBallY[i] + BALL_RADIUS >= SCREEN_HEIGHT) {
                extraBallsActive[i] = false;
                player.HP -= 1;
            }

            // Paddle collision
            Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
            if (CheckCollisionCircleRec((Vector2){extraBallX[i], extraBallY[i]}, BALL_RADIUS, playerRect)) {
                extraBallSpeedY[i] = -BALL_SPEED;
                float hitPos = (extraBallX[i] - playerX) / (SCREEN_WIDTH / 20.0f);
                extraBallSpeedX[i] = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
            }

            // Check block collisions
            CheckBlockCollision(&extraBallX[i], &extraBallY[i], BALL_RADIUS,
                                &extraBallSpeedX[i], &extraBallSpeedY[i]);
        }
    }

    // Paddle movement
    if (IsKeyDown(KEY_A) && playerX > 0) {
        playerX -= movementSpeed;
    }
    else if (IsKeyDown(KEY_D) && playerX + (SCREEN_WIDTH / 20) < SCREEN_WIDTH) {
        playerX += movementSpeed;
    }

    // Check if player is out of lives
    if (player.HP <= 0) {
        player.HP = 0;
        isAlive = false;
        ball_active = false;
        for (int i = 0; i < 4; i++) {
            extraBallsActive[i] = false;
        }
    }

    // Check if all blocks are cleared
    if (AllBlocksCleared() && !gameWon) {
        gameWon = true;
        ball_active = false;
        for (int i = 0; i < 4; i++) {
            extraBallsActive[i] = false;
        }
    }
}

// ----------------------------------------------------------------------
//  Draw all game elements
// ----------------------------------------------------------------------
void DrawGame(void) {
    // Score
    DrawText(TextFormat("%.0f", player.currentScore),
             SCREEN_WIDTH/2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

    // Highscore
    if (player.currentScore > player.highscore) {
        player.highscore = player.currentScore;
    }
    DrawText(TextFormat("Highscore: %.0f", player.highscore),
             SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);

    // Lives
    DrawText(TextFormat("Lives: %.0f", player.HP),
             SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);

    // Paddle
    DrawRectangle((int)playerX, (int)playerY, SCREEN_WIDTH / 20, SCREEN_HEIGHT / 50, WHITE);

    // Main ball
    if (ball_active) {
        DrawCircle((int)ballX, (int)ballY, BALL_RADIUS, WHITE);
    }

    // Extra balls
    for (int i = 0; i < 4; i++) {
        if (extraBallsActive[i]) {
            DrawCircle((int)extraBallX[i], (int)extraBallY[i], BALL_RADIUS, YELLOW);
        }
    }

    // Blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            DrawRectangleRec(blocks[i].rect, blocks[i].color);
            DrawText(TextFormat("%d", blocks[i].health),
                     (int)(blocks[i].rect.x + blocks[i].rect.width/2 - 10),
                     (int)(blocks[i].rect.y + blocks[i].rect.height/2 - 10),
                     20, WHITE);
        }
    }
}

// ----------------------------------------------------------------------
//  Win Screen (if all blocks cleared)
// ----------------------------------------------------------------------
void WinScreen(void) {
    if (IsKeyPressed(KEY_Y)) {
        level.currentRows *= 2; // Doubling rows for next level
        GameStarter();
    }
    else if (IsKeyPressed(KEY_N)) {
        CloseWindow();
    }
    levelReset();

    DrawText("YOU WIN!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 50, GREEN);
    DrawText("GENERATE NEXT LEVEL (Y/N)", SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 + 60, 50, WHITE);
}

// ----------------------------------------------------------------------
//  GameOver screen
// ----------------------------------------------------------------------
void GameOver(void) {
    if (IsKeyPressed(KEY_Y)) {
        GameStarter();
    }
    else if (IsKeyPressed(KEY_N)) {
        dataLoader(false);
        CloseWindow();
    }

    DrawText("GAME OVER!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 50, RED);
    DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 + 60, 50, WHITE);
}

// ----------------------------------------------------------------------
//  Resets the rows if they get too large
// ----------------------------------------------------------------------
void levelReset(void) {
    if (level.currentRows >= 14) {
        level.currentRows = ROWS;
    }
}

// ----------------------------------------------------------------------
//  Manages saving and loading user highscore data
// ----------------------------------------------------------------------
void dataLoader(bool load) {
    if (load) {
        FILE *file = fopen("highscore.txt", "r");
        if (file) {
            float storedHighscore = 0.0f;
            if (fscanf(file, "%f", &storedHighscore) == 1) {
                player.highscore = storedHighscore;
            }
            fclose(file);
        } else {
            player.highscore = 0.0f;
        }
    } else {
        FILE *file = fopen("highscore.txt", "w");
        if (file) {
            fprintf(file, "%.0f\n", player.highscore);
            fclose(file);
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block kuzushi raylib game build");
    SetTargetFPS(60);

    dataLoader(true);

    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;

    // Main loop
    while (!WindowShouldClose())
    {
        GameState();

        BeginDrawing();
        ClearBackground(BLACK);

        switch (currentState)
        {
            case NOT_STARTED:
                InitializeGame();
                break;

            case GAME_OVER:
                GameOver();
                break;

            case GAME_WIN:
                WinScreen();
                break;

            case GAME_PLAYING:
                Upgrades();
                UpdateGame();
                DrawGame();
                break;

            default:
                break;
        }
        EndDrawing();
    }
    dataLoader(false);
    CloseWindow();
    return 0;
}