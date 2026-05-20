#pragma once

#include "Constants.h"
#include "Structs.h"
#include <GL/glut.h>

extern GameState gameState;
extern CameraMode cameraMode;

extern char username[USERNAME_MAX];
extern int usernameLen;
extern bool loginError;
extern char loginMsg[64];

extern int menuSelection;
extern const int MENU_COUNT;
extern const char* menuItems[];
extern bool showControls;

extern int playerLane;
extern float playerX;
extern float playerY;
extern float playerVelY;
extern bool isJumping;
extern bool isSliding;
extern float slideTimer;
extern float legAngle;
extern float armAngle;
extern float squashY;
extern float squashTimer;
extern float playerXSmooth;

extern float worldOffset;
extern float gameSpeed;
extern float speedTimer;
extern float topSpeed;
extern float envHue;
extern float introTimer;

extern int score;
extern int coinCount;
extern float scoreTimer;

extern Obstacle obstacles[MAX_OBSTACLES];
extern Coin coins[MAX_COINS];
extern Building buildings[MAX_BUILDINGS];
extern Lamp lamps[MAX_LAMPS];
extern Cloud clouds[MAX_CLOUDS];

extern GLuint buildingTexture;
extern float characterAngle;
