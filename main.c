#include <stdio.h>
#include "raylib.h"

#define SCREEN_WIDTH   1800
#define SCREEN_HEIGHT  900
#define ROWS           8
#define COLUMNS        14
#define MAX_BLOCKS     (ROWS * COLUMNS)
#define BLOCK_WIDTH    100
#define BLOCK_HEIGHT   30
#define BLOCK_SPACING  10

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

static ScoreData player = {3, 0.0f, 0.0f};
static Block blocks[MAX_BLOCKS];
static float playerX;
static float playerY;
static float movementSpeed = 50.0f;
static bool  bulletActive = false;
static float ballX, ballY;
static float ballSpeedX, ballSpeedY;
static const float BALL_SPEED  = 10.0f;
static const float BALL_RADIUS = 8.0f;
static bool gameStarted = false;
static bool isAlive     = true;
static const int KONAMI_CODE[] = {
    KEY_UP, KEY_UP, KEY_DOWN, KEY_DOWN,
    KEY_LEFT, KEY_RIGHT, KEY_LEFT, KEY_RIGHT,
    KEY_B, KEY_A
};
static const int KONAMI_CODE_LENGTH = 10;
static int konamiIndex = 0;
static bool  fourBallsSpawned  = false;
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

void InitializeBlocks(void) {
    int blockIndex = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
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

void GameStarter(void) {
    player.healthPoints = 3;
    player.currentScore = 0;
    isAlive             = true;
    gameStarted         = true;
    InitializeBlocks();
    fourBallsSpawned = false;
    playerX = SCREEN_WIDTH / 2.0f;
    playerY = SCREEN_HEIGHT - 150.0f;
    bulletActive = true;
    ballX        = playerX + 40.0f;
    ballY        = playerY - 40.0f;
    ballSpeedX   = BALL_SPEED;
    ballSpeedY   = -BALL_SPEED;
    for (int i = 0; i < 4; i++) extraBulletActive[i] = false;
}

void InitializeGame(void) {
    if (!gameStarted) {
        DrawText("START GAME (Y/N)", SCREEN_WIDTH/2 - 250, SCREEN_HEIGHT/2, 50, WHITE);
        int key = GetKeyPressed();
        switch (key) {
            case KEY_Y: GameStarter(); break;
            case KEY_N: CloseWindow(); break;
            default: break;
        }
    }
}

bool IsAnyKeyPressed(void) {
    for (int key = KEY_SPACE; key <= KEY_KP_9; key++) {
        if (IsKeyPressed(key)) return true;
    }
    return false;
}

void Upgrades(void) {
    if (IsKeyPressed(KONAMI_CODE[konamiIndex])) {
        konamiIndex++;
        if (konamiIndex == KONAMI_CODE_LENGTH) {
            player.healthPoints = 9001;
            konamiIndex = 0;
        }
    }
    else if (IsAnyKeyPressed()) {
        konamiIndex = 0;
    }
}

void CheckBallBlockCollision(void) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, blocks[i].rect)) {
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            ballSpeedY *= -1.0f;
            break;
        }
    }
}

static void CheckExtraBallsBlockCollision(int index) {
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active &&
            CheckCollisionCircleRec((Vector2){ extraBallX[index], extraBallY[index] }, BALL_RADIUS, blocks[i].rect)) {
            blocks[i].health--;
            if (blocks[i].health <= 0) {
                blocks[i].active = false;
                player.currentScore += 100;
            }
            extraBallSpeedY[index] *= -1.0f;
            break;
        }
    }
}

static void SpawnFourBallsIfNeeded(void) {
    if (!fourBallsSpawned && player.currentScore >= 4000.0f) {
        for (int i = 0; i < 4; i++) {
            extraBulletActive[i] = true;
            extraBallX[i] = ballX;
            extraBallY[i] = ballY;
            float angle = GetRandomValue(0, 359) * DEG2RAD;
            extraBallSpeedX[i] = cosf(angle) * BALL_SPEED;
            extraBallSpeedY[i] = sinf(angle) * BALL_SPEED;
        }
        fourBallsSpawned = true;
    }
}

void UpdateGame(void) {
    if (IsKeyPressed(KEY_SPACE) && !bulletActive && isAlive) {
        bulletActive = true;
        ballX = playerX + (SCREEN_WIDTH / 50);
        ballY = playerY;
        ballSpeedY = -BALL_SPEED;
        ballSpeedX = (GetRandomValue(0, 1) == 0) ? -BALL_SPEED / 2 : BALL_SPEED / 2;
    }
    if (bulletActive) {
        ballX += ballSpeedX;
        ballY += ballSpeedY;
        if (ballX - BALL_RADIUS <= 0 || ballX + BALL_RADIUS >= SCREEN_WIDTH) ballSpeedX *= -1;
        if (ballY - BALL_RADIUS <= 0) ballSpeedY *= -1;
        if (ballY + BALL_RADIUS >= SCREEN_HEIGHT) {
            bulletActive = false;
            player.healthPoints -= 1;
        }
        Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
        if (CheckCollisionCircleRec((Vector2){ballX, ballY}, BALL_RADIUS, playerRect)) {
            ballSpeedY = -BALL_SPEED;
            float hitPos = (ballX - playerX) / (SCREEN_WIDTH / 20.0f);
            ballSpeedX = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
        }
        CheckBallBlockCollision();
    }
    SpawnFourBallsIfNeeded();
    for (int i = 0; i < 4; i++) {
        if (extraBulletActive[i]) {
            extraBallX[i] += extraBallSpeedX[i];
            extraBallY[i] += extraBallSpeedY[i];
            if (extraBallX[i] - BALL_RADIUS <= 0 || extraBallX[i] + BALL_RADIUS >= SCREEN_WIDTH) extraBallSpeedX[i] *= -1.0f;
            if (extraBallY[i] - BALL_RADIUS <= 0) extraBallSpeedY[i] *= -1.0f;
            if (extraBallY[i] + BALL_RADIUS >= SCREEN_HEIGHT) {
                extraBulletActive[i] = false;
                player.healthPoints -= 1;
            }
            Rectangle playerRect = { playerX, playerY, SCREEN_WIDTH / 20.0f, SCREEN_HEIGHT / 50.0f };
            if (CheckCollisionCircleRec((Vector2){ extraBallX[i], extraBallY[i] }, BALL_RADIUS, playerRect)) {
                extraBallSpeedY[i] = -BALL_SPEED;
                float hitPos = (extraBallX[i] - playerX) / (SCREEN_WIDTH / 20.0f);
                extraBallSpeedX[i] = (hitPos - 0.5f) * BALL_SPEED * 2.0f;
            }
            CheckExtraBallsBlockCollision(i);
        }
    }
    if (IsKeyDown(KEY_A) && playerX > 0) playerX -= movementSpeed;
    else if (IsKeyDown(KEY_D) && playerX + (SCREEN_WIDTH/20) < SCREEN_WIDTH) playerX += movementSpeed;
    if (player.healthPoints <= 0) {
        player.healthPoints = 0;
        isAlive = false;
        bulletActive = false;
        for (int i = 0; i < 4; i++) extraBulletActive[i] = false;
    }
}

void DrawGame(void) {
    DrawText(TextFormat("%.0f", player.currentScore), SCREEN_WIDTH/2 - 155, SCREEN_HEIGHT - 100, 50, WHITE);

    if (player.currentScore > player.highscore) {
        player.highscore = player.currentScore;
    }
    DrawText(TextFormat("Highscore: %.0f", player.highscore), SCREEN_WIDTH - 400, SCREEN_HEIGHT - 100, 50, WHITE);

    DrawText(TextFormat("Lives: %.0f", player.healthPoints), SCREEN_WIDTH -1700, SCREEN_HEIGHT - 100, 50, WHITE);
    DrawRectangle((int)playerX, (int)playerY, SCREEN_WIDTH/20, SCREEN_HEIGHT/50, WHITE);
    if (bulletActive) DrawCircle((int)ballX, (int)ballY, BALL_RADIUS, WHITE);
    for (int i = 0; i < 4; i++) {
        if (extraBulletActive[i]) DrawCircle((int)extraBallX[i], (int)extraBallY[i], BALL_RADIUS, WHITE);
    }
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

void GameOver(void) {
    DrawText("GAME OVER!", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2, 50, RED);
    DrawText("RESTART GAME (Y/N)", SCREEN_WIDTH/2 - 300, SCREEN_HEIGHT/2 + 60, 50, WHITE);
    if (IsKeyPressed(KEY_Y)) GameStarter();
    else if (IsKeyPressed(KEY_N)) CloseWindow();
}

int main(void) {
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
