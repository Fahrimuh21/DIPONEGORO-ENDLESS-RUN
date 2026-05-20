#include "../header/Globals.h"
#include "../header/Game.h"
#include "../header/Init.h"
#include "../header/Utility.h"
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

GameState gameState = INTRO;
CameraMode cameraMode = CAM_BACK;

char username[USERNAME_MAX] = "";
int usernameLen = 0;
bool loginError = false;
char loginMsg[64] = "";

int menuSelection = 0;
const int MENU_COUNT = 4;
const char* menuItems[] = {
    "Start Game",
    "Camera Mode",
    "Controls",
    "Exit"
};
bool showControls = false;

int playerLane = 0;
float playerX = 0.0f;
float playerY = 0.0f;
float playerVelY = 0.0f;
bool isJumping = false;
bool isSliding = false;
float slideTimer = 0.0f;
float legAngle = 0.0f;
float armAngle = 0.0f;
float squashY = 1.0f;
float squashTimer = 0.0f;
float playerXSmooth = 0.0f;

float worldOffset = 0.0f;
float gameSpeed = 0.55f;
float speedTimer = 0.0f;
float topSpeed = 0.55f;
float envHue = 0.0f;
float introTimer = 0.0f;

int score = 0;
int coinCount = 0;
float scoreTimer = 0.0f;

Obstacle obstacles[MAX_OBSTACLES];
Coin coins[MAX_COINS];
Building buildings[MAX_BUILDINGS];
Lamp lamps[MAX_LAMPS];
Cloud clouds[MAX_CLOUDS];

GLuint buildingTexture;
float characterAngle = 0.0f;

void initGame() {
    playerLane = 0;
    playerX = 0.0f;
    playerY = 0.0f;
    playerVelY = 0.0f;
    isJumping = false;
    isSliding = false;
    slideTimer = 0.0f;
    legAngle = 0.0f;
    armAngle = 0.0f;
    squashY = 1.0f;
    squashTimer = 0.0f;
    playerXSmooth = 0.0f;

    worldOffset = 0.0f;
    gameSpeed = 0.55f;
    speedTimer = 0.0f;
    topSpeed = gameSpeed;
    score = 0;
    coinCount = 0;
    scoreTimer = 0.0f;

    initObstacles();
    initCoins();
    initBuildings();
    initLamps();
    initClouds();
}

bool checkObstacleCollision(Obstacle& o) {
    if (!o.active) return false;
    if (o.z < 3.8f || o.z > 7.4f) return false;
    if (o.lane != playerLane) return false;
    if (isJumping && playerY > 1.15f) return false;
    if (isSliding && o.type == 2) return false;
    return true;
}

void update(int value) {
    introTimer += 0.016f;

    if (gameState == PLAYING) {
        worldOffset += gameSpeed;
        envHue = fmod(envHue + 0.0009f, 1.0f);
        speedTimer += 0.016f;
        scoreTimer += 0.016f;

        if (speedTimer > 3.0f) {
            speedTimer = 0.0f;
            gameSpeed += 0.025f;
            if (gameSpeed > topSpeed) topSpeed = gameSpeed;
        }

        if (scoreTimer > 0.15f) {
            scoreTimer = 0.0f;
            score++;
        }

        playerX = lerp(playerX, laneX(playerLane), 0.22f);
        legAngle += 0.22f + gameSpeed * 0.12f;
        armAngle += 0.24f + gameSpeed * 0.12f;

        if (isJumping) {
            playerY += playerVelY;
            playerVelY -= 0.018f;
            if (playerY <= 0.0f) {
                playerY = 0.0f;
                playerVelY = 0.0f;
                isJumping = false;
            }
        }

        if (isSliding) {
            slideTimer -= 0.016f;
            if (slideTimer <= 0.0f) {
                slideTimer = 0.0f;
                isSliding = false;
            }
        }

        squashY = isSliding ? 0.55f : 1.0f;

        for (int i = 0; i < MAX_OBSTACLES; i++) {
            obstacles[i].z += gameSpeed;
            if (checkObstacleCollision(obstacles[i])) {
                gameState = GAMEOVER;
            }
            if (obstacles[i].z > 18.0f) {
                obstacles[i].z = -165.0f - (float)(rand() % 30);
                obstacles[i].lane = (rand() % 3) - 1;
                obstacles[i].type = rand() % 4;
                obstacles[i].active = true;
                obstacles[i].length = 6.0f + (rand() % 4);
                obstacles[i].color = rand() % 4;
            }
        }

        for (int i = 0; i < MAX_COINS; i++) {
            coins[i].z += gameSpeed;
            coins[i].rot += 6.0f;
            if (coins[i].active && coins[i].z > 4.5f && coins[i].z < 7.0f && fabs(coins[i].x - laneX(playerLane)) < 0.8f) {
                coins[i].active = false;
                coinCount++;
                score += 10;
            }
            if (coins[i].z > 18.0f) {
                coins[i].z = -170.0f - (float)(rand() % 40);
                coins[i].x = laneX((rand() % 3) - 1);
                coins[i].active = true;
                coins[i].rot = 0.0f;
            }
        }

        for (int i = 0; i < MAX_CLOUDS; i++) {
            clouds[i].x += clouds[i].speed;
            if (clouds[i].x > 38.0f) clouds[i].x = -38.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}
