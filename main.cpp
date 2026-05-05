// ============================================================
//   SUBWAY RUNNER - OpenGL/GLUT Endless Runner
//   Subway Surfers-style 3D game
//   Compile: g++ subway_runner.cpp -o subway_runner -lGL -lGLU -lglut -lm
//   Windows: g++ subway_runner.cpp -o subway_runner -lopengl32 -lglu32 -lfreeglut
// ============================================================

#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
//  CONSTANTS
// ============================================================
#define WINDOW_W 800
#define WINDOW_H 600
#define LANE_LEFT  -3.5f
#define LANE_MID    0.0f
#define LANE_RIGHT  3.5f
#define GROUND_Y   -2.0f
#define MAX_OBSTACLES 6
#define MAX_COINS     12
#define MAX_BUILDINGS 16

// ============================================================
//  GAME STATE
// ============================================================
enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;

// ============================================================
//  PLAYER
// ============================================================
int   playerLane    = 0;       // -1, 0, 1
float playerX       = 0.0f;
float playerY       = 0.0f;    // relative to ground
float playerVelY    = 0.0f;
bool  isJumping     = false;
bool  isSliding     = false;
float slideTimer    = 0.0f;
float legAngle      = 0.0f;
float armAngle      = 0.0f;
float squashY       = 1.0f;    // land squash effect
float squashTimer   = 0.0f;

// ============================================================
//  WORLD
// ============================================================
float worldOffset   = 0.0f;
float gameSpeed     = 0.55f;
float speedTimer    = 0.0f;

// Environment color cycle
float envHue        = 0.0f;    // 0..1 cycles through time-of-day

// ============================================================
//  OBSTACLES
// ============================================================
struct Obstacle {
    float z;
    int   lane;     // -1,0,1
    int   type;     // 0=barrier, 1=train car, 2=low beam
    bool  active;
};
Obstacle obstacles[MAX_OBSTACLES];

// ============================================================
//  COINS
// ============================================================
struct Coin {
    float x, y, z;
    bool  active;
    float rot;
};
Coin coins[MAX_COINS];

// ============================================================
//  BUILDINGS
// ============================================================
struct Building {
    float z;
    float h;        // height
    float w;        // width
    int   side;     // -1 left, 1 right
    float r, g, b;
};
Building buildings[MAX_BUILDINGS];

// ============================================================
//  CLOUDS
// ============================================================
struct Cloud {
    float x, y, z;
    float speed;
    float scale;
};
#define MAX_CLOUDS 8
Cloud clouds[MAX_CLOUDS];

// ============================================================
//  SCORE / COINS
// ============================================================
int   score      = 0;
int   coinCount  = 0;
float scoreTimer = 0.0f;

// ============================================================
//  UTILITY
// ============================================================
float laneX(int l) {
    if (l == -1) return LANE_LEFT;
    if (l ==  1) return LANE_RIGHT;
    return LANE_MID;
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

// HSV -> RGB helper
void hsvToRgb(float h, float s, float v, float &r, float &g, float &b) {
    int   i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r=v; g=t; b=p; break;
        case 1: r=q; g=v; b=p; break;
        case 2: r=p; g=v; b=t; break;
        case 3: r=p; g=q; b=v; break;
        case 4: r=t; g=p; b=v; break;
        case 5: r=v; g=p; b=q; break;
    }
}

// ============================================================
//  DRAW TEXT (2D Overlay)
// ============================================================
void begin2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

void end2D() {
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

void drawTextCentered(float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    int len = glutBitmapLength(font, (const unsigned char*)text);
    drawText((WINDOW_W - len) * 0.5f, y, text, font);
}

// ============================================================
//  DRAW PANEL
// ============================================================
void drawPanel(float x1, float y1, float x2, float y2,
               float r=0, float g=0, float b=0, float a=0.72f) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
    // border
    glColor4f(1,1,1,0.25f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1); glVertex2f(x2, y1);
    glVertex2f(x2, y2); glVertex2f(x1, y2);
    glEnd();
}

// ============================================================
//  INIT GAME OBJECTS
// ============================================================
void initObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].z    = -60.0f - i * 22.0f;
        obstacles[i].lane = (rand() % 3) - 1;
        obstacles[i].type = rand() % 3;
        obstacles[i].active = true;
    }
}

void initCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].z      = -40.0f - i * 15.0f;
        coins[i].x      = laneX((rand() % 3) - 1);
        coins[i].y      = GROUND_Y + 1.5f;
        coins[i].active = true;
        coins[i].rot    = 0;
    }
}

void initBuildings() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        float h = 8.0f + (rand() % 14);
        float w = 3.5f + (rand() % 3);
        int   side = (i % 2 == 0) ? -1 : 1;
        float r2, g2, b2;
        hsvToRgb((float)(rand() % 100) / 100.0f, 0.4f, 0.85f, r2, g2, b2);
        buildings[i] = { -10.0f - (i / 2) * 9.0f, h, w, side, r2, g2, b2 };
    }
}

void initClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i] = {
            -30.0f + (float)(rand() % 60),
            15.0f  + (float)(rand() % 8),
            -20.0f - (float)(rand() % 40),
            0.02f  + (float)(rand() % 3) * 0.01f,
            1.5f   + (float)(rand() % 3)
        };
    }
}

void initGame() {
    playerLane  = 0;
    playerX     = 0.0f;
    playerY     = 0.0f;
    playerVelY  = 0.0f;
    isJumping   = false;
    isSliding   = false;
    slideTimer  = 0.0f;
    legAngle    = 0.0f;
    worldOffset = 0.0f;
    gameSpeed   = 0.55f;
    speedTimer  = 0.0f;
    score       = 0;
    coinCount   = 0;
    scoreTimer  = 0.0f;
    envHue      = 0.0f;
    squashY     = 1.0f;
    squashTimer = 0.0f;

    initObstacles();
    initCoins();
    initBuildings();
    initClouds();
}

// ============================================================
//  DRAW SKY (gradient)
// ============================================================
void drawSky() {
    // top colour from env cycle
    float sr, sg, sb;
    hsvToRgb(fmod(envHue + 0.55f, 1.0f), 0.7f, 0.9f, sr, sg, sb);
    float hr, hg, hb;
    hsvToRgb(fmod(envHue + 0.08f, 1.0f), 0.5f, 1.0f, hr, hg, hb);

    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glColor3f(sr, sg, sb);
    glVertex3f(-200, 80, -150);
    glVertex3f( 200, 80, -150);
    glColor3f(hr, hg, hb);
    glVertex3f( 200,  0, -150);
    glVertex3f(-200,  0, -150);
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

// ============================================================
//  DRAW CLOUDS
// ============================================================
void drawClouds() {
    glDisable(GL_DEPTH_TEST);
    glColor3f(1, 1, 1);
    for (int i = 0; i < MAX_CLOUDS; i++) {
        float cx = clouds[i].x;
        float cy = clouds[i].y;
        float cz = clouds[i].z;
        float sc = clouds[i].scale;

        glPushMatrix();
        glTranslatef(cx, cy, cz);
        glScalef(sc * 3, sc, sc * 1.5f);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(cx + sc, cy + 0.4f, cz);
        glScalef(sc * 2, sc * 0.8f, sc);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(cx - sc, cy + 0.3f, cz);
        glScalef(sc * 1.5f, sc * 0.7f, sc);
        glutSolidCube(1);
        glPopMatrix();
    }
    glEnable(GL_DEPTH_TEST);
}

// ============================================================
//  DRAW GROUND (tiled track)
// ============================================================
void drawGround() {
    // Side walls / rails
    float railColor1[3] = {0.8f, 0.6f, 0.1f};

    for (int i = 0; i < 22; i++) {
        float z = 12.0f - i * 10.0f + fmod(worldOffset, 10.0f);

        // Main road
        if (i % 2 == 0)
            glColor3f(0.35f, 0.33f, 0.38f);
        else
            glColor3f(0.30f, 0.28f, 0.32f);

        glBegin(GL_QUADS);
        glVertex3f(-6, GROUND_Y, z);
        glVertex3f( 6, GROUND_Y, z);
        glVertex3f( 6, GROUND_Y, z - 10);
        glVertex3f(-6, GROUND_Y, z - 10);
        glEnd();

        // Lane dividers
        glColor3f(1.0f, 0.85f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-3.6f, GROUND_Y + 0.01f, z);
        glVertex3f(-3.4f, GROUND_Y + 0.01f, z);
        glVertex3f(-3.4f, GROUND_Y + 0.01f, z - 4);
        glVertex3f(-3.6f, GROUND_Y + 0.01f, z - 4);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3f( 3.4f, GROUND_Y + 0.01f, z);
        glVertex3f( 3.6f, GROUND_Y + 0.01f, z);
        glVertex3f( 3.6f, GROUND_Y + 0.01f, z - 4);
        glVertex3f( 3.4f, GROUND_Y + 0.01f, z - 4);
        glEnd();

        // Rail tracks (subway style)
        glColor3f(railColor1[0], railColor1[1], railColor1[2]);
        // Left track
        glBegin(GL_QUADS);
        glVertex3f(-5.9f, GROUND_Y + 0.02f, z);
        glVertex3f(-5.6f, GROUND_Y + 0.02f, z);
        glVertex3f(-5.6f, GROUND_Y + 0.02f, z - 10);
        glVertex3f(-5.9f, GROUND_Y + 0.02f, z - 10);
        glEnd();
        // Right track
        glBegin(GL_QUADS);
        glVertex3f( 5.6f, GROUND_Y + 0.02f, z);
        glVertex3f( 5.9f, GROUND_Y + 0.02f, z);
        glVertex3f( 5.9f, GROUND_Y + 0.02f, z - 10);
        glVertex3f( 5.6f, GROUND_Y + 0.02f, z - 10);
        glEnd();

        // Cross sleepers
        glColor3f(0.45f, 0.3f, 0.15f);
        for (int j = 0; j < 4; j++) {
            float sz = z - j * 2.5f;
            glBegin(GL_QUADS);
            glVertex3f(-6.1f, GROUND_Y + 0.01f, sz);
            glVertex3f( 6.1f, GROUND_Y + 0.01f, sz);
            glVertex3f( 6.1f, GROUND_Y + 0.01f, sz - 0.3f);
            glVertex3f(-6.1f, GROUND_Y + 0.01f, sz - 0.3f);
            glEnd();
        }
    }

    // Platform edges
    glColor3f(0.7f, 0.7f, 0.75f);
    glBegin(GL_QUADS);
    glVertex3f(-7, GROUND_Y, 15);
    glVertex3f(-6, GROUND_Y, 15);
    glVertex3f(-6, GROUND_Y, -200);
    glVertex3f(-7, GROUND_Y, -200);
    glEnd();
    glBegin(GL_QUADS);
    glVertex3f( 6, GROUND_Y, 15);
    glVertex3f( 7, GROUND_Y, 15);
    glVertex3f( 7, GROUND_Y, -200);
    glVertex3f( 6, GROUND_Y, -200);
    glEnd();
}

// ============================================================
//  DRAW BUILDINGS
// ============================================================
void drawBuildings() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building& b = buildings[i];
        float bx = b.side * (7.5f + b.w * 0.5f);
        float by = GROUND_Y + b.h * 0.5f;
        float bz = b.z + fmod(worldOffset, 9.0f);

        // Recycle buildings
        if (bz > 15.0f) {
            b.z -= MAX_BUILDINGS * 9.0f * 0.5f + 5.0f;
            float h2 = 8.0f + (rand() % 14);
            float w2 = 3.5f + (rand() % 3);
            b.h = h2; b.w = w2;
            hsvToRgb((float)(rand() % 100) / 100.0f, 0.4f, 0.85f, b.r, b.g, b.b);
            continue;
        }

        // Building body
        glColor3f(b.r, b.g, b.b);
        glPushMatrix();
        glTranslatef(bx, by, bz);
        glScalef(b.w, b.h, 5.0f);
        glutSolidCube(1);
        glPopMatrix();

        // Windows grid
        glColor3f(
            fmin(b.r + 0.3f, 1.0f),
            fmin(b.g + 0.3f, 1.0f),
            fmin(b.b + 0.3f, 1.0f)
        );
        int floors = (int)(b.h / 2.0f);
        int wcols  = (int)(b.w / 1.5f);
        for (int fy = 0; fy < floors; fy++) {
            for (int fx = 0; fx < wcols; fx++) {
                float wx = bx - b.w * 0.4f + fx * (b.w * 0.8f / (wcols > 1 ? wcols - 1 : 1));
                float wy = GROUND_Y + 0.8f + fy * 2.0f;
                float wz = bz + 2.6f;
                glPushMatrix();
                glTranslatef(wx, wy, wz);
                glScalef(0.6f, 0.8f, 0.1f);
                glutSolidCube(1);
                glPopMatrix();
            }
        }

        // Rooftop detail
        glColor3f(b.r * 0.7f, b.g * 0.7f, b.b * 0.7f);
        glPushMatrix();
        glTranslatef(bx, GROUND_Y + b.h + 0.5f, bz);
        glScalef(b.w * 0.6f, 1.0f, 3.0f);
        glutSolidCube(1);
        glPopMatrix();

        // Billboard on some buildings
        if (i % 4 == 0) {
            glColor3f(1.0f, 0.3f, 0.1f);
            glPushMatrix();
            glTranslatef(bx, GROUND_Y + b.h + 1.5f, bz);
            glScalef(b.w * 0.8f, 1.5f, 0.2f);
            glutSolidCube(1);
            glPopMatrix();
        }
    }
}

// ============================================================
//  DRAW PLAYER
// ============================================================
void drawPlayer() {
    float slideScale = isSliding ? 0.5f : 1.0f;
    float bodyH      = 1.6f * squashY * slideScale;
    float headY      = isSliding ? 0.3f : 1.5f;

    glPushMatrix();
    glTranslatef(playerX, GROUND_Y + playerY + 1.2f, 5.5f);

    // BODY
    glColor3f(0.1f, 0.6f, 1.0f);   // blue hoodie
    glPushMatrix();
    glScalef(1.3f, bodyH, 0.9f);
    glutSolidCube(1);
    glPopMatrix();

    // HEAD
    glColor3f(1.0f, 0.82f, 0.65f);
    glPushMatrix();
    glTranslatef(0, headY * squashY, 0);
    glScalef(0.9f, 0.9f, 0.9f);
    glutSolidCube(1);
    glPopMatrix();

    // CAP / HAT
    glColor3f(1.0f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0, headY * squashY + 0.55f, 0.1f);
    glScalef(1.0f, 0.25f, 0.85f);
    glutSolidCube(1);
    glPopMatrix();

    if (!isSliding) {
        // LEFT ARM
        glColor3f(0.1f, 0.6f, 1.0f);
        glPushMatrix();
        glTranslatef(-1.0f, 0.3f, 0);
        glRotatef(sinf(armAngle) * 50.0f, 1, 0, 0);
        glTranslatef(0, -0.7f, 0);
        glScalef(0.45f, 1.4f, 0.45f);
        glutSolidCube(1);
        glPopMatrix();

        // RIGHT ARM
        glPushMatrix();
        glTranslatef(1.0f, 0.3f, 0);
        glRotatef(-sinf(armAngle) * 50.0f, 1, 0, 0);
        glTranslatef(0, -0.7f, 0);
        glScalef(0.45f, 1.4f, 0.45f);
        glutSolidCube(1);
        glPopMatrix();

        // LEFT LEG
        glColor3f(0.15f, 0.15f, 0.35f);
        glPushMatrix();
        glTranslatef(-0.4f, -0.8f, 0);
        glRotatef(sinf(legAngle) * 55.0f, 1, 0, 0);
        glTranslatef(0, -0.7f, 0);
        glScalef(0.55f, 1.5f, 0.55f);
        glutSolidCube(1);
        glPopMatrix();

        // RIGHT LEG
        glPushMatrix();
        glTranslatef(0.4f, -0.8f, 0);
        glRotatef(-sinf(legAngle) * 55.0f, 1, 0, 0);
        glTranslatef(0, -0.7f, 0);
        glScalef(0.55f, 1.5f, 0.55f);
        glutSolidCube(1);
        glPopMatrix();

        // SHOES
        glColor3f(0.9f, 0.85f, 0.1f);
        glPushMatrix();
        glTranslatef(-0.4f, -2.0f, sinf(legAngle) * 0.3f);
        glScalef(0.6f, 0.35f, 0.9f);
        glutSolidCube(1);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.4f, -2.0f, -sinf(legAngle) * 0.3f);
        glScalef(0.6f, 0.35f, 0.9f);
        glutSolidCube(1);
        glPopMatrix();
    }

    glPopMatrix();
}

// ============================================================
//  DRAW OBSTACLE
// ============================================================
void drawObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        Obstacle& o = obstacles[i];
        float ox = laneX(o.lane);
        float oz = o.z;

        glPushMatrix();
        glTranslatef(ox, GROUND_Y, oz);

        switch (o.type) {
            case 0: // BARRIER (red/white)
                glColor3f(0.95f, 0.1f, 0.1f);
                glPushMatrix();
                glTranslatef(0, 1.2f, 0);
                glScalef(3.0f, 2.4f, 0.6f);
                glutSolidCube(1);
                glPopMatrix();
                // stripes
                glColor3f(1.0f, 1.0f, 1.0f);
                for (int s = 0; s < 3; s++) {
                    glPushMatrix();
                    glTranslatef(-0.8f + s * 0.8f, 1.2f, 0.31f);
                    glScalef(0.3f, 2.4f, 0.05f);
                    glutSolidCube(1);
                    glPopMatrix();
                }
                break;

            case 1: // TRAIN CAR
                glColor3f(0.2f, 0.5f, 0.9f);
                glPushMatrix();
                glTranslatef(0, 1.8f, 0);
                glScalef(3.0f, 3.6f, 6.0f);
                glutSolidCube(1);
                glPopMatrix();
                // windows
                glColor3f(0.8f, 0.95f, 1.0f);
                for (int w = 0; w < 2; w++) {
                    glPushMatrix();
                    glTranslatef(-0.7f + w * 1.4f, 2.5f, 3.1f);
                    glScalef(0.7f, 1.0f, 0.1f);
                    glutSolidCube(1);
                    glPopMatrix();
                }
                break;

            case 2: // LOW BEAM (duck under)
                glColor3f(0.9f, 0.6f, 0.1f);
                // Left pole
                glPushMatrix();
                glTranslatef(-1.5f, 1.0f, 0);
                glScalef(0.3f, 2.0f, 0.3f);
                glutSolidCube(1);
                glPopMatrix();
                // Right pole
                glPushMatrix();
                glTranslatef(1.5f, 1.0f, 0);
                glScalef(0.3f, 2.0f, 0.3f);
                glutSolidCube(1);
                glPopMatrix();
                // Horizontal beam
                glColor3f(1.0f, 0.3f, 0.1f);
                glPushMatrix();
                glTranslatef(0, 2.1f, 0);
                glScalef(3.3f, 0.35f, 0.35f);
                glutSolidCube(1);
                glPopMatrix();
                break;
        }
        glPopMatrix();
    }
}

// ============================================================
//  DRAW COINS
// ============================================================
void drawCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].active) continue;
        Coin& c = coins[i];

        glPushMatrix();
        glTranslatef(c.x, c.y, c.z);
        glRotatef(c.rot, 0, 1, 0);

        // Coin body
        glColor3f(1.0f, 0.85f, 0.1f);
        glPushMatrix();
        glScalef(0.6f, 0.6f, 0.15f);
        glutSolidCube(1);
        glPopMatrix();

        // Highlight
        glColor3f(1.0f, 1.0f, 0.6f);
        glPushMatrix();
        glTranslatef(0.05f, 0.05f, 0.08f);
        glScalef(0.25f, 0.25f, 0.05f);
        glutSolidCube(1);
        glPopMatrix();

        glPopMatrix();
    }
}

// ============================================================
//  DRAW UI
// ============================================================
void drawUI() {
    begin2D();

    if (gameState == START) {
        // Background panel
        drawPanel(200, 170, 600, 440, 0.05f, 0.05f, 0.15f, 0.85f);

        // Title
        glColor3f(1.0f, 0.85f, 0.1f);
        drawTextCentered(395, "SUBWAY RUNNER", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(0.9f, 0.9f, 0.9f);
        drawTextCentered(350, "Press SPACE to Start", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.7f, 0.9f, 1.0f);
        drawTextCentered(315, "LEFT / RIGHT ARROW  :  Change Lane", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(295, "SPACE               :  Jump", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(275, "S  or  DOWN ARROW   :  Slide", GLUT_BITMAP_HELVETICA_12);

        glColor3f(1.0f, 0.5f, 0.5f);
        drawTextCentered(230, "Avoid obstacles. Collect coins!", GLUT_BITMAP_HELVETICA_12);
    }
    else if (gameState == PLAYING) {
        // Score bar background
        drawPanel(0, 565, 800, 600, 0, 0, 0, 0.5f);

        char buf[64];
        sprintf(buf, "Score: %d", score);
        glColor3f(1.0f, 0.85f, 0.1f);
        drawText(20, 575, buf, GLUT_BITMAP_HELVETICA_18);

        sprintf(buf, "Coins: %d", coinCount);
        glColor3f(1.0f, 1.0f, 0.4f);
        drawText(200, 575, buf, GLUT_BITMAP_HELVETICA_18);

        // Speed indicator
        sprintf(buf, "Speed: x%.1f", gameSpeed / 0.55f);
        glColor3f(0.5f, 1.0f, 0.6f);
        drawText(380, 575, buf, GLUT_BITMAP_HELVETICA_12);

        // Jump hint
        if (isSliding) {
            glColor3f(0.5f, 1.0f, 1.0f);
            drawTextCentered(40, "SLIDING!", GLUT_BITMAP_HELVETICA_18);
        }
    }
    else if (gameState == GAMEOVER) {
        drawPanel(175, 150, 625, 460, 0.1f, 0.0f, 0.0f, 0.9f);

        glColor3f(1.0f, 0.2f, 0.2f);
        drawTextCentered(405, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

        char buf[64];
        glColor3f(1.0f, 0.85f, 0.1f);
        sprintf(buf, "Score: %d", score);
        drawTextCentered(360, buf, GLUT_BITMAP_HELVETICA_18);

        sprintf(buf, "Coins Collected: %d", coinCount);
        glColor3f(1.0f, 1.0f, 0.4f);
        drawTextCentered(330, buf, GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.8f, 0.8f, 0.8f);
        sprintf(buf, "Top Speed: x%.1f", gameSpeed / 0.55f);
        drawTextCentered(295, buf, GLUT_BITMAP_HELVETICA_12);

        glColor3f(0.4f, 1.0f, 0.4f);
        drawTextCentered(245, "Press R to Restart", GLUT_BITMAP_HELVETICA_18);

        glColor3f(0.6f, 0.6f, 0.6f);
        drawTextCentered(210, "Press ESC to Quit", GLUT_BITMAP_HELVETICA_12);
    }

    end2D();
}

// ============================================================
//  DISPLAY
// ============================================================
void display() {
    // Sky clear colour
    float sr, sg, sb;
    hsvToRgb(fmod(envHue + 0.55f, 1.0f), 0.4f, 0.7f, sr, sg, sb);
    glClearColor(sr, sg, sb, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    drawSky();
    drawClouds();

    // Camera: follow player with slight downward tilt
    float camX = playerX * 0.5f;
    gluLookAt(
        camX, 5.5f, 14.0f,   // eye
        playerX * 0.3f, 0.5f, -5.0f,  // center
        0, 1, 0
    );

    drawBuildings();
    drawGround();
    drawCoins();
    drawObstacles();
    drawPlayer();

    drawUI();

    glutSwapBuffers();
}

// ============================================================
//  COLLISION DETECTION
// ============================================================
bool checkObstacleCollision(Obstacle& o) {
    float px = playerX;
    float pz = 5.5f;

    float ox = laneX(o.lane);
    float oz = o.z;

    float dx = fabs(px - ox);
    float dz = fabs(pz - oz);

    if (dx > 2.0f || dz > 2.5f) return false;

    // Type 2 = low beam: player can slide under
    if (o.type == 2 && isSliding) return false;
    // Type 2: must duck, if not sliding -> hit
    if (o.type == 2 && playerY < 0.5f && !isSliding) return true;
    if (o.type == 2 && playerY >= 0.5f) return false; // jumped over

    // If player jumped above obstacle
    if (playerY > 2.5f) return false;

    return true;
}

// ============================================================
//  UPDATE
// ============================================================
void update(int value) {

    if (gameState == PLAYING) {

        // --- Speed ramp-up ---
        speedTimer += 0.013f;
        gameSpeed = 0.55f + speedTimer * 0.012f;
        if (gameSpeed > 1.6f) gameSpeed = 1.6f;

        // --- Env colour shift ---
        envHue += 0.00015f;
        if (envHue > 1.0f) envHue -= 1.0f;

        // --- Animation ---
        legAngle += gameSpeed * 7.0f;
        armAngle  = legAngle;

        // --- Squash recover ---
        if (squashTimer > 0) {
            squashTimer -= 0.05f;
            squashY = lerp(1.0f, 0.5f, squashTimer);
        } else { squashY = 1.0f; }

        // --- Lane movement ---
        float targetX = laneX(playerLane);
        playerX = lerp(playerX, targetX, 0.18f);

        // --- Jump physics ---
        if (isJumping) {
            playerY    += playerVelY;
            playerVelY -= 0.012f;
            if (playerY <= 0.0f) {
                playerY     = 0.0f;
                isJumping   = false;
                playerVelY  = 0.0f;
                squashY     = 0.5f;
                squashTimer = 1.0f;
            }
        }

        // --- Slide timer ---
        if (isSliding) {
            slideTimer -= 0.025f;
            if (slideTimer <= 0) isSliding = false;
        }

        // --- World scroll ---
        worldOffset += gameSpeed;

        // --- Score ---
        scoreTimer += gameSpeed;
        if (scoreTimer >= 10.0f) {
            score++;
            scoreTimer = 0.0f;
        }

        // --- Update obstacles ---
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            obstacles[i].z += gameSpeed;

            if (obstacles[i].z > 12.0f) {
                obstacles[i].z    = -55.0f - (rand() % 20);
                obstacles[i].lane = (rand() % 3) - 1;
                obstacles[i].type = rand() % 3;
                obstacles[i].active = true;
            }

            if (obstacles[i].active && checkObstacleCollision(obstacles[i])) {
                gameState = GAMEOVER;
            }
        }

        // --- Update coins ---
        for (int i = 0; i < MAX_COINS; i++) {
            if (!coins[i].active) continue;
            coins[i].z   += gameSpeed;
            coins[i].rot += 4.0f;

            if (coins[i].z > 12.0f) {
                coins[i].z      = -40.0f - (rand() % 30);
                coins[i].x      = laneX((rand() % 3) - 1);
                coins[i].y      = GROUND_Y + 1.5f;
                coins[i].active = true;
            }

            // Coin collection
            float dx = fabs(playerX - coins[i].x);
            float dz = fabs(5.5f   - coins[i].z);
            float dy = fabs((GROUND_Y + playerY + 1.2f) - coins[i].y);
            if (dx < 1.2f && dz < 1.5f && dy < 1.5f) {
                coins[i].active = false;
                coinCount++;
                score += 5;
            }
        }

        // --- Update clouds ---
        for (int i = 0; i < MAX_CLOUDS; i++) {
            clouds[i].x += clouds[i].speed;
            if (clouds[i].x > 50) clouds[i].x = -50;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(13, update, 0);
}

// ============================================================
//  INPUT
// ============================================================
void keySpecial(int key, int x, int y) {
    if (gameState != PLAYING) return;

    if (key == GLUT_KEY_LEFT  && playerLane > -1) playerLane--;
    if (key == GLUT_KEY_RIGHT && playerLane <  1) playerLane++;

    if (key == GLUT_KEY_DOWN && !isJumping) {
        isSliding  = true;
        slideTimer = 1.0f;
    }
}

void keyNormal(unsigned char key, int x, int y) {
    if (key == 27) exit(0);  // ESC

    if (gameState == START) {
        if (key == ' ') gameState = PLAYING;
        return;
    }

    if (gameState == GAMEOVER) {
        if (key == 'r' || key == 'R') {
            initGame();
            gameState = PLAYING;
        }
        return;
    }

    // PLAYING
    if (key == ' ' && !isJumping && !isSliding) {
        isJumping  = true;
        playerVelY = 0.28f;
    }
    if ((key == 's' || key == 'S') && !isJumping) {
        isSliding  = true;
        slideTimer = 1.0f;
    }
    if (key == 'a' || key == 'A') { if (playerLane > -1) playerLane--; }
    if (key == 'd' || key == 'D') { if (playerLane <  1) playerLane++; }
}

// ============================================================
//  RESHAPE
// ============================================================
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / h, 0.5, 300.0);
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
//  INIT GL
// ============================================================
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Simple lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPos[]  = { 5, 20, 5, 1 };
    GLfloat lightAmb[]  = { 0.4f, 0.4f, 0.45f, 1 };
    GLfloat lightDiff[] = { 1.0f, 0.95f, 0.9f, 1 };
    GLfloat lightSpec[] = { 0.6f, 0.6f, 0.6f, 1 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0 / 600.0, 0.5, 300.0);
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
//  MAIN
// ============================================================
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(100, 80);
    glutCreateWindow("Subway Runner - OpenGL");

    initGL();
    initGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keySpecial);
    glutKeyboardFunc(keyNormal);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
