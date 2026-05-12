// ============================================================
//   SUBWAY RUNNER - Enhanced OpenGL/GLUT Endless Runner
//   Versi pengembangan dengan: Intro/Login, Multi-Camera,
//   Visual realistis, Obstacle kereta detail, UI profesional
//
//   Compile Linux:
//     g++ subway_runner.cpp -o subway_runner -lGL -lGLU -lglut -lm
//   Compile Windows:
//     g++ subway_runner.cpp -o subway_runner -lopengl32 -lglu32 -lfreeglut -lm
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
#define WINDOW_W      900
#define WINDOW_H      600
#define LANE_LEFT    -2.5f
#define LANE_MID      0.0f
#define LANE_RIGHT    2.5f
#define LANE_WIDTH    2.5f
#define GROUND_Y     -2.0f
#define MAX_OBSTACLES  8
#define MAX_COINS     14
#define MAX_BUILDINGS 20
#define MAX_CLOUDS     8
#define MAX_LAMPS      8
#define USERNAME_MAX  32

// ============================================================
//  ENUMS
// ============================================================
enum GameState  { INTRO, LOGIN, MENU, PLAYING, PAUSED, GAMEOVER };
enum CameraMode { CAM_BACK, CAM_SIDE, CAM_FIRST };

GameState  gameState  = INTRO;
CameraMode cameraMode = CAM_BACK;

// ============================================================
//  USERNAME / LOGIN
// ============================================================
char  username[USERNAME_MAX] = "";
int   usernameLen = 0;
bool  loginError  = false;
char  loginMsg[64] = "";

// ============================================================
//  MENU
// ============================================================
int  menuSelection = 0;
// 0=Start Game, 1=Camera Mode, 2=Controls, 3=Exit
const int MENU_COUNT = 4;
const char* menuItems[MENU_COUNT] = {
    "Start Game",
    "Camera Mode",
    "Controls",
    "Exit"
};
bool showControls = false;

// ============================================================
//  PLAYER
// ============================================================
int   playerLane   = 0;
float playerX      = 0.0f;
float playerY      = 0.0f;
float playerVelY   = 0.0f;
bool  isJumping    = false;
bool  isSliding    = false;
float slideTimer   = 0.0f;
float legAngle     = 0.0f;
float armAngle     = 0.0f;
float squashY      = 1.0f;
float squashTimer  = 0.0f;
float playerXSmooth= 0.0f;

// ============================================================
//  WORLD
// ============================================================
float worldOffset  = 0.0f;
float gameSpeed    = 0.55f;
float speedTimer   = 0.0f;
float topSpeed     = 0.55f;
float envHue       = 0.0f;
float introTimer   = 0.0f;  // for intro animation

// ============================================================
//  SCORE
// ============================================================
int   score      = 0;
int   coinCount  = 0;
float scoreTimer = 0.0f;

// ============================================================
//  OBSTACLES
// ============================================================
struct Obstacle {
    float z;
    int   lane;      // -1,0,1  (lane==99 = full-track train)
    int   type;      // 0=barrier, 1=train, 2=lowbeam, 3=long_train, 4=double
    bool  active;
    float length;    // for type==3 long train
    int   color;     // train color variant
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
    float z, h, w;
    int   side;
    float r, g, b;
};
Building buildings[MAX_BUILDINGS];

// ============================================================
//  STREET LAMPS
// ============================================================
struct Lamp {
    float z;
    int   side;
};
Lamp lamps[MAX_LAMPS];

// ============================================================
//  CLOUDS
// ============================================================
struct Cloud {
    float x, y, z, speed, scale;
};
Cloud clouds[MAX_CLOUDS];

// ============================================================
//  UTILITY
// ============================================================
float laneX(int l) {
    if (l == -1) return LANE_LEFT;
    if (l ==  1) return LANE_RIGHT;
    return LANE_MID;
}
float lerp(float a, float b, float t) { return a + (b - a) * t; }
float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void hsvToRgb(float h, float s, float v, float &r, float &g, float &b) {
    int   i = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t2= v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r=v;  g=t2; b=p;  break;
        case 1: r=q;  g=v;  b=p;  break;
        case 2: r=p;  g=v;  b=t2; break;
        case 3: r=p;  g=q;  b=v;  break;
        case 4: r=t2; g=p;  b=v;  break;
        case 5: r=v;  g=p;  b=q;  break;
    }
}

// ============================================================
//  2D OVERLAY HELPERS
// ============================================================
void begin2D() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}

void end2D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawText(float x, float y, const char* text,
              void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++)
        glutBitmapCharacter(font, text[i]);
}

int textWidth(const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    return glutBitmapLength(font, (const unsigned char*)text);
}

void drawTextCentered(float y, const char* text,
                      void* font = GLUT_BITMAP_HELVETICA_18) {
    float x = (WINDOW_W - textWidth(text, font)) * 0.5f;
    drawText(x, y, text, font);
}

void drawPanel(float x1, float y1, float x2, float y2,
               float r=0, float g=0, float b=0, float a=0.78f,
               bool border=true) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
    if (border) {
        glColor4f(1,1,1,0.3f);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x1,y1); glVertex2f(x2,y1);
        glVertex2f(x2,y2); glVertex2f(x1,y2);
        glEnd();
    }
}

// Highlighted menu item button
void drawMenuButton(float cx, float cy, float w, float h,
                    const char* label, bool selected, void* font=GLUT_BITMAP_HELVETICA_18) {
    float x1 = cx - w*0.5f, x2 = cx + w*0.5f;
    float y1 = cy - h*0.5f, y2 = cy + h*0.5f;

    if (selected) {
        glColor4f(0.2f, 0.6f, 1.0f, 0.85f);
    } else {
        glColor4f(0.1f, 0.1f, 0.2f, 0.7f);
    }
    glBegin(GL_QUADS);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();
    glColor4f(selected?1.0f:0.5f, selected?1.0f:0.5f, 1.0f, 0.6f);
    glLineWidth(selected?2.0f:1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1,y1); glVertex2f(x2,y1);
    glVertex2f(x2,y2); glVertex2f(x1,y2);
    glEnd();

    if (selected)
        glColor3f(1.0f, 1.0f, 1.0f);
    else
        glColor3f(0.75f, 0.75f, 0.85f);
    float tx = cx - textWidth(label, font)*0.5f;
    drawText(tx, cy - 6, label, font);
}

// ============================================================
//  DRAW CYLINDER (for lamp poles, train wheels, etc.)
// ============================================================
void drawCylinder(float radius, float height, int slices=10) {
    GLUquadric* q = gluNewQuadric();
    glPushMatrix();
    glRotatef(-90,1,0,0);
    gluCylinder(q, radius, radius, height, slices, 1);
    gluDisk(q, 0, radius, slices, 1);
    glTranslatef(0,0,height);
    gluDisk(q, 0, radius, slices, 1);
    glPopMatrix();
    gluDeleteQuadric(q);
}

void drawSphere(float radius, int slices=10, int stacks=8) {
    GLUquadric* q = gluNewQuadric();
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}

// ============================================================
//  INIT
// ============================================================
void initObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].z      = -65.0f - i * 25.0f;
        obstacles[i].lane   = (rand() % 3) - 1;
        obstacles[i].type   = rand() % 4;
        obstacles[i].active = true;
        obstacles[i].length = 6.0f + (rand() % 4);
        obstacles[i].color  = rand() % 4;
    }
}

void initCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].z      = -45.0f - i * 14.0f;
        coins[i].x      = laneX((rand() % 3) - 1);
        coins[i].y      = GROUND_Y + 1.5f;
        coins[i].active = true;
        coins[i].rot    = 0;
    }
}

void initBuildings() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        float h = 7.0f + (rand() % 16);
        float w = 3.0f + (rand() % 4);
        int side = (i % 2 == 0) ? -1 : 1;
        float r2,g2,b2;
        hsvToRgb((float)(rand()%100)/100.0f, 0.35f, 0.80f, r2,g2,b2);
        buildings[i] = { -12.0f - (i/2)*10.0f, h, w, side, r2,g2,b2 };
    }
}

void initLamps() {
    for (int i = 0; i < MAX_LAMPS; i++) {
        lamps[i].z    = -5.0f - i * 16.0f;
        lamps[i].side = (i % 2 == 0) ? -1 : 1;
    }
}

void initClouds() {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i] = {
            -30.0f + (float)(rand()%60),
            14.0f  + (float)(rand()%8),
            -20.0f - (float)(rand()%40),
            0.015f + (float)(rand()%3)*0.008f,
            1.5f   + (float)(rand()%3)
        };
    }
}

void initGame() {
    playerLane   = 0;
    playerX      = 0.0f;
    playerXSmooth= 0.0f;
    playerY      = 0.0f;
    playerVelY   = 0.0f;
    isJumping    = false;
    isSliding    = false;
    slideTimer   = 0.0f;
    legAngle     = 0.0f;
    worldOffset  = 0.0f;
    gameSpeed    = 0.55f;
    speedTimer   = 0.0f;
    topSpeed     = 0.55f;
    score        = 0;
    coinCount    = 0;
    scoreTimer   = 0.0f;
    envHue       = 0.22f;
    squashY      = 1.0f;
    squashTimer  = 0.0f;
    cameraMode   = CAM_BACK;
    initObstacles();
    initCoins();
    initBuildings();
    initLamps();
    initClouds();
}

// ============================================================
//  DRAW SKY
// ============================================================
void drawSky() {
    float sr,sg,sb, hr,hg,hb;
    hsvToRgb(fmod(envHue+0.55f,1.0f), 0.7f, 0.85f, sr,sg,sb);
    hsvToRgb(fmod(envHue+0.10f,1.0f), 0.5f, 1.0f,  hr,hg,hb);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3f(sr,sg,sb); glVertex3f(-300,80,-200); glVertex3f(300,80,-200);
    glColor3f(hr,hg,hb); glVertex3f(300, -5,-200); glVertex3f(-300,-5,-200);
    glEnd();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ============================================================
//  DRAW CLOUDS
// ============================================================
void drawClouds() {
    glDisable(GL_LIGHTING);
    glColor3f(1,1,1);
    for (int i = 0; i < MAX_CLOUDS; i++) {
        float cx=clouds[i].x, cy=clouds[i].y, cz=clouds[i].z, sc=clouds[i].scale;
        glPushMatrix(); glTranslatef(cx,cy,cz); glScalef(sc*3,sc,sc*1.5f); glutSolidCube(1); glPopMatrix();
        glPushMatrix(); glTranslatef(cx+sc,cy+0.4f,cz); glScalef(sc*2,sc*0.8f,sc); glutSolidCube(1); glPopMatrix();
        glPushMatrix(); glTranslatef(cx-sc,cy+0.3f,cz); glScalef(sc*1.5f,sc*0.7f,sc); glutSolidCube(1); glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}

// ============================================================
//  DRAW REALISTIC GROUND
// ============================================================
void drawRealisticGround() {
    // 3 lanes: left(-2.5), mid(0), right(+2.5)
    // Each lane width = 2.5, total track = -3.75 to +3.75
    // Lane boundaries: left wall=-3.75, divider1=-1.25, divider2=+1.25, right wall=+3.75
    const float TL = -3.75f;  // track left edge
    const float TR =  3.75f;  // track right edge
    const float D1 = -1.25f;  // divider between left and mid lane
    const float D2 =  1.25f;  // divider between mid and right lane

    for (int i = 0; i < 28; i++) {
        float z  = 15.0f - i*10.0f + fmod(worldOffset, 10.0f);
        float z2 = z - 10.0f;

        glDisable(GL_LIGHTING);

        // --- 3 Lane floor tiles ---
        // Left lane
        float s = (i%2==0) ? 0.33f : 0.28f;
        glColor3f(s, s-0.02f, s+0.04f);
        glBegin(GL_QUADS);
        glVertex3f(TL,GROUND_Y,z); glVertex3f(D1,GROUND_Y,z);
        glVertex3f(D1,GROUND_Y,z2); glVertex3f(TL,GROUND_Y,z2);
        glEnd();

        // Mid lane (slightly lighter)
        s = (i%2==0) ? 0.37f : 0.31f;
        glColor3f(s, s-0.02f, s+0.04f);
        glBegin(GL_QUADS);
        glVertex3f(D1,GROUND_Y,z); glVertex3f(D2,GROUND_Y,z);
        glVertex3f(D2,GROUND_Y,z2); glVertex3f(D1,GROUND_Y,z2);
        glEnd();

        // Right lane
        s = (i%2==0) ? 0.33f : 0.28f;
        glColor3f(s, s-0.02f, s+0.04f);
        glBegin(GL_QUADS);
        glVertex3f(D2,GROUND_Y,z); glVertex3f(TR,GROUND_Y,z);
        glVertex3f(TR,GROUND_Y,z2); glVertex3f(D2,GROUND_Y,z2);
        glEnd();

        // --- Lane divider lines (dashed white, 2 dashes per 10-unit tile) ---
        glColor3f(0.92f, 0.92f, 0.92f);
        for (int d = 0; d < 2; d++) {
            float dx = (d==0) ? D1 : D2;
            for (int dash = 0; dash < 2; dash++) {
                float dz1 = z    - dash*5.0f;
                float dz2 = dz1  - 3.2f;
                glBegin(GL_QUADS);
                glVertex3f(dx-0.07f, GROUND_Y+0.02f, dz1);
                glVertex3f(dx+0.07f, GROUND_Y+0.02f, dz1);
                glVertex3f(dx+0.07f, GROUND_Y+0.02f, dz2);
                glVertex3f(dx-0.07f, GROUND_Y+0.02f, dz2);
                glEnd();
            }
        }

        // --- Outer border lines (solid yellow) ---
        glColor3f(0.95f, 0.80f, 0.0f);
        for (int s2=0; s2<2; s2++) {
            float bx = (s2==0) ? TL : TR - 0.12f;
            glBegin(GL_QUADS);
            glVertex3f(bx,       GROUND_Y+0.02f, z);
            glVertex3f(bx+0.12f, GROUND_Y+0.02f, z);
            glVertex3f(bx+0.12f, GROUND_Y+0.02f, z2);
            glVertex3f(bx,       GROUND_Y+0.02f, z2);
            glEnd();
        }

        // --- Rail tracks (2 rails, one near each outer edge) ---
        float railX[2] = { TL + 0.35f, TR - 0.57f };
        for (int r=0; r<2; r++) {
            float rx = railX[r];
            glColor3f(0.72f, 0.68f, 0.62f);
            glBegin(GL_QUADS);
            glVertex3f(rx,       GROUND_Y+0.04f, z);
            glVertex3f(rx+0.22f, GROUND_Y+0.04f, z);
            glVertex3f(rx+0.22f, GROUND_Y+0.04f, z2);
            glVertex3f(rx,       GROUND_Y+0.04f, z2);
            glEnd();
            // shiny top
            glColor3f(0.88f, 0.85f, 0.78f);
            glBegin(GL_QUADS);
            glVertex3f(rx+0.04f, GROUND_Y+0.06f, z);
            glVertex3f(rx+0.18f, GROUND_Y+0.06f, z);
            glVertex3f(rx+0.18f, GROUND_Y+0.06f, z2);
            glVertex3f(rx+0.04f, GROUND_Y+0.06f, z2);
            glEnd();
        }

        // --- Sleepers (wooden cross-ties) ---
        for (int j=0; j<5; j++) {
            float sz = z - j*2.0f - 0.5f;
            glColor3f(0.42f, 0.27f, 0.14f);
            glBegin(GL_QUADS);
            glVertex3f(TL-0.2f, GROUND_Y+0.02f, sz);
            glVertex3f(TR+0.2f, GROUND_Y+0.02f, sz);
            glVertex3f(TR+0.2f, GROUND_Y+0.02f, sz-0.24f);
            glVertex3f(TL-0.2f, GROUND_Y+0.02f, sz-0.24f);
            glEnd();
            glColor3f(0.28f, 0.18f, 0.08f);
            glBegin(GL_QUADS);
            glVertex3f(TL-0.2f, GROUND_Y+0.01f, sz-0.24f);
            glVertex3f(TR+0.2f, GROUND_Y+0.01f, sz-0.24f);
            glVertex3f(TR+0.2f, GROUND_Y+0.01f, sz-0.32f);
            glVertex3f(TL-0.2f, GROUND_Y+0.01f, sz-0.32f);
            glEnd();
        }

        glEnable(GL_LIGHTING);

        // --- Platform / sidewalk both sides ---
        glColor3f(0.68f, 0.68f, 0.72f);
        glBegin(GL_QUADS);
        glVertex3f(TL-4.0f,GROUND_Y,z);  glVertex3f(TL,GROUND_Y,z);
        glVertex3f(TL,      GROUND_Y,z2); glVertex3f(TL-4.0f,GROUND_Y,z2);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3f(TR,      GROUND_Y,z);  glVertex3f(TR+4.0f,GROUND_Y,z);
        glVertex3f(TR+4.0f, GROUND_Y,z2); glVertex3f(TR,     GROUND_Y,z2);
        glEnd();

        // Yellow safety strip on platform edge
        glDisable(GL_LIGHTING);
        glColor3f(0.95f, 0.85f, 0.08f);
        glBegin(GL_QUADS);
        glVertex3f(TL-0.22f,GROUND_Y+0.03f,z); glVertex3f(TL,GROUND_Y+0.03f,z);
        glVertex3f(TL,      GROUND_Y+0.03f,z2); glVertex3f(TL-0.22f,GROUND_Y+0.03f,z2);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3f(TR,      GROUND_Y+0.03f,z); glVertex3f(TR+0.22f,GROUND_Y+0.03f,z);
        glVertex3f(TR+0.22f,GROUND_Y+0.03f,z2); glVertex3f(TR,     GROUND_Y+0.03f,z2);
        glEnd();

        // Player shadow - draw only once (tile 0)
        if (i == 0) {
            glColor4f(0,0,0,0.28f);
            float srad = isSliding ? 1.0f : 0.75f;
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(playerX, GROUND_Y+0.01f, 5.5f);
            for (int k=0;k<=18;k++) {
                float ang = k*(2*3.14159265f/18);
                glVertex3f(playerX+cosf(ang)*srad, GROUND_Y+0.01f, 5.5f+sinf(ang)*0.5f);
            }
            glEnd();
        }

        glEnable(GL_LIGHTING);
    }

    // Far tunnel wall
    glDisable(GL_LIGHTING);
    glColor3f(0.15f,0.15f,0.20f);
    glBegin(GL_QUADS);
    glVertex3f(-10,12,-200); glVertex3f(10,12,-200);
    glVertex3f(10,GROUND_Y,-200); glVertex3f(-10,GROUND_Y,-200);
    glEnd();
    glEnable(GL_LIGHTING);
}

// ============================================================
//  DRAW STREET LAMPS
// ============================================================
//  DRAW STREET LAMPS
// ============================================================
void drawStreetLamps() {
    for (int i = 0; i < MAX_LAMPS; i++) {
        float lz = lamps[i].z + fmod(worldOffset, 16.0f);
        float lx = lamps[i].side * 8.5f;

        if (lz > 16.0f) { lamps[i].z -= MAX_LAMPS * 16.0f; continue; }

        // Pole
        glColor3f(0.55f,0.55f,0.6f);
        glPushMatrix();
        glTranslatef(lx, GROUND_Y, lz);
        drawCylinder(0.12f, 7.0f, 8);
        glPopMatrix();

        // Arm
        glColor3f(0.55f,0.55f,0.6f);
        glPushMatrix();
        float armDir = lamps[i].side * -1.0f;
        glTranslatef(lx + armDir*0.8f, GROUND_Y+6.5f, lz);
        glScalef(1.6f, 0.12f, 0.12f);
        glutSolidCube(1);
        glPopMatrix();

        // Lamp head
        glColor3f(1.0f, 0.92f, 0.6f);
        glPushMatrix();
        glTranslatef(lx + armDir*1.5f, GROUND_Y+6.4f, lz);
        glScalef(0.6f, 0.22f, 0.35f);
        glutSolidCube(1);
        glPopMatrix();

        // Glow halo
        glDisable(GL_LIGHTING);
        glColor4f(1.0f, 0.95f, 0.5f, 0.18f);
        glPushMatrix();
        glTranslatef(lx + armDir*1.5f, GROUND_Y+6.3f, lz);
        drawSphere(0.6f, 10, 8);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}

// ============================================================
//  DRAW BUILDINGS
// ============================================================
void drawBuildings() {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building& b = buildings[i];
        float bx = b.side * (9.5f + b.w*0.5f);
        float by = GROUND_Y + b.h*0.5f;
        float bz = b.z + fmod(worldOffset, 10.0f);

        if (bz > 16.0f) {
            b.z -= MAX_BUILDINGS*10.0f*0.5f + 5.0f;
            b.h  = 7.0f + (rand()%16);
            b.w  = 3.0f + (rand()%4);
            hsvToRgb((float)(rand()%100)/100.0f,0.35f,0.80f,b.r,b.g,b.b);
            continue;
        }

        glColor3f(b.r, b.g, b.b);
        glPushMatrix(); glTranslatef(bx,by,bz); glScalef(b.w, b.h, 5.5f); glutSolidCube(1); glPopMatrix();

        // Windows
        int floors = (int)(b.h/2.2f);
        int wcols  = (int)(b.w/1.4f);
        for (int fy=0; fy<floors; fy++) {
            for (int fx=0; fx<wcols; fx++) {
                float wx = bx - b.w*0.38f + fx*(b.w*0.76f/(wcols>1?wcols-1:1));
                float wy = GROUND_Y + 0.9f + fy*2.2f;
                float wz = bz + 2.8f;
                bool lit = (rand()%5) != 0;
                if (lit)
                    glColor3f(1.0f, 0.97f, 0.7f);
                else
                    glColor3f(0.25f,0.28f,0.32f);
                glPushMatrix(); glTranslatef(wx,wy,wz); glScalef(0.55f,0.75f,0.08f); glutSolidCube(1); glPopMatrix();
            }
        }

        // Roof
        glColor3f(b.r*0.65f, b.g*0.65f, b.b*0.65f);
        glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+0.4f,bz); glScalef(b.w*0.6f,0.8f,3.5f); glutSolidCube(1); glPopMatrix();

        // Billboard every 4th building
        if (i%4 == 0) {
            glColor3f(1.0f, 0.2f, 0.05f);
            glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+1.6f,bz); glScalef(b.w*0.85f,1.6f,0.2f); glutSolidCube(1); glPopMatrix();
            glColor3f(1,1,0.3f);
            glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+1.65f,bz+0.12f); glScalef(b.w*0.5f,0.6f,0.05f); glutSolidCube(1); glPopMatrix();
        }
    }
}

// ============================================================
//  DRAW FOG
// ============================================================
void enableFog() {
    float sr,sg,sb;
    hsvToRgb(fmod(envHue+0.08f,1.0f),0.4f,0.75f,sr,sg,sb);
    GLfloat fogColor[4] = {sr,sg,sb,1.0f};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 40.0f);
    glFogf(GL_FOG_END,  130.0f);
    glEnable(GL_FOG);
}

// ============================================================
//  DRAW PLAYER
// ============================================================
void drawPlayer() {
    float slideScale = isSliding ? 0.5f : 1.0f;
    float bodyH = 1.6f * squashY * slideScale;
    float headY = isSliding ? 0.3f : 1.55f;

    glPushMatrix();
    glTranslatef(playerX, GROUND_Y + playerY + 1.25f, 5.5f);
    glRotatef(180.0f, 0, 1, 0);  // face forward (-Z direction)

    // Body (hoodie)
    glColor3f(0.08f, 0.52f, 0.95f);
    glPushMatrix(); glScalef(1.25f, bodyH, 0.85f); glutSolidCube(1); glPopMatrix();

    // Belt / waist stripe
    glColor3f(0.95f, 0.2f, 0.1f);
    glPushMatrix(); glTranslatef(0, -0.6f*squashY*slideScale, 0); glScalef(1.27f, 0.15f, 0.86f); glutSolidCube(1); glPopMatrix();

    // Head
    glColor3f(0.98f, 0.80f, 0.62f);
    glPushMatrix(); glTranslatef(0, headY*squashY, 0); glScalef(0.88f,0.88f,0.88f); glutSolidCube(1); glPopMatrix();

    // Eyes
    glColor3f(0.1f,0.1f,0.15f);
    for (int e=-1; e<=1; e+=2) {
        glPushMatrix();
        glTranslatef(e*0.2f, headY*squashY+0.05f, -0.45f);
        drawSphere(0.08f,5,4);
        glPopMatrix();
    }

    // Cap brim
    glColor3f(0.9f,0.08f,0.08f);
    glPushMatrix(); glTranslatef(0, headY*squashY+0.52f, -0.12f); glScalef(0.92f,0.22f,0.8f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(0, headY*squashY+0.4f,  -0.5f); glScalef(0.55f,0.12f,0.3f); glutSolidCube(1); glPopMatrix();

    if (!isSliding) {
        // Arms
        glColor3f(0.08f, 0.52f, 0.95f);
        for (int side=-1; side<=1; side+=2) {
            glPushMatrix();
            glTranslatef(side*0.95f, 0.3f, 0);
            glRotatef(sinf(armAngle) * side * -50.0f, 1,0,0);
            glTranslatef(0,-0.7f,0);
            glScalef(0.42f,1.35f,0.42f);
            glutSolidCube(1);
            glPopMatrix();
        }

        // Legs
        glColor3f(0.15f,0.15f,0.32f);
        for (int leg=-1; leg<=1; leg+=2) {
            glPushMatrix();
            glTranslatef(leg*0.38f, -0.82f, 0);
            glRotatef(sinf(legAngle)*leg*-55.0f, 1,0,0);
            glTranslatef(0,-0.72f,0);
            glScalef(0.52f,1.45f,0.52f);
            glutSolidCube(1);
            glPopMatrix();
        }

        // Shoes
        glColor3f(0.9f,0.82f,0.08f);
        for (int s=-1; s<=1; s+=2) {
            glPushMatrix();
            glTranslatef(s*0.38f, -2.02f, sinf(legAngle)*s*0.25f);
            glScalef(0.58f,0.32f,0.88f);
            glutSolidCube(1);
            glPopMatrix();
        }
    } else {
        // Slide: arms out
        glColor3f(0.08f, 0.52f, 0.95f);
        for (int side=-1; side<=1; side+=2) {
            glPushMatrix();
            glTranslatef(side*1.1f, 0.1f, 0);
            glScalef(0.42f,0.42f,1.3f);
            glutSolidCube(1);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

// ============================================================
//  DRAW TRAIN OBSTACLE (detailed)
// ============================================================
void drawTrainObstacle(float ox, float oz, int colorVar, float len) {
    // Color variants
    float trainColors[4][3] = {
        {0.18f,0.42f,0.85f},  // blue
        {0.7f, 0.1f, 0.1f},   // red
        {0.2f, 0.55f,0.25f},  // green
        {0.4f, 0.4f, 0.45f}   // grey
    };
    float r = trainColors[colorVar][0];
    float g = trainColors[colorVar][1];
    float b = trainColors[colorVar][2];

    glPushMatrix();
    glTranslatef(ox, GROUND_Y, oz);

    // Main body
    glColor3f(r,g,b);
    glPushMatrix(); glTranslatef(0,2.0f,0); glScalef(2.1f, 4.0f, len); glutSolidCube(1); glPopMatrix();

    // Roof stripe
    glColor3f(r*0.6f, g*0.6f, b*0.6f);
    glPushMatrix(); glTranslatef(0,4.05f,0); glScalef(2.2f,0.2f,len); glutSolidCube(1); glPopMatrix();

    // Side stripe
    glColor3f(fminf(r+0.3f,1.0f),fminf(g+0.3f,1.0f),fminf(b+0.3f,1.0f));
    glPushMatrix(); glTranslatef(0,2.3f,len*0.5f+0.01f); glScalef(2.12f,0.35f,0.05f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(0,2.3f,-len*0.5f-0.01f); glScalef(2.12f,0.35f,0.05f); glutSolidCube(1); glPopMatrix();

    // Windows (front face)
    glColor3f(0.75f,0.92f,1.0f);
    for (int w=0; w<3; w++) {
        glPushMatrix();
        glTranslatef(-0.6f+w*0.6f, 2.5f, len*0.5f+0.05f);
        glScalef(0.45f, 0.85f, 0.08f);
        glutSolidCube(1);
        glPopMatrix();
        // Window frame
        glColor3f(0.25f,0.25f,0.28f);
        glPushMatrix();
        glTranslatef(-0.6f+w*0.6f, 2.5f, len*0.5f+0.06f);
        glScalef(0.52f,0.9f,0.06f);
        glutSolidCube(1);
        glPopMatrix();
        glColor3f(0.75f,0.92f,1.0f);
    }

    // Front headlights
    glColor3f(1.0f, 0.97f, 0.7f);
    for (int h=-1; h<=1; h+=2) {
        glPushMatrix();
        glTranslatef(h*0.75f, 1.2f, len*0.5f+0.06f);
        drawSphere(0.22f,8,6);
        glPopMatrix();
        // Glow
        glDisable(GL_LIGHTING);
        glColor4f(1.0f,0.95f,0.5f,0.22f);
        glPushMatrix();
        glTranslatef(h*0.75f, 1.2f, len*0.5f+0.12f);
        drawSphere(0.55f,8,6);
        glPopMatrix();
        glEnable(GL_LIGHTING);
        glColor3f(1.0f,0.97f,0.7f);
    }

    // Door
    glColor3f(r*0.78f, g*0.78f, b*0.78f);
    glPushMatrix(); glTranslatef(0,1.8f,len*0.5f+0.05f); glScalef(0.7f,2.8f,0.06f); glutSolidCube(1); glPopMatrix();
    // Door handle
    glColor3f(0.8f,0.8f,0.8f);
    glPushMatrix(); glTranslatef(0.25f,1.8f,len*0.5f+0.1f); glScalef(0.07f,0.55f,0.07f); glutSolidCube(1); glPopMatrix();

    // Bogies / wheels (2 sets)
    glColor3f(0.2f,0.2f,0.22f);
    for (int bw=0; bw<2; bw++) {
        float bz = (bw==0) ? len*0.3f : -len*0.3f;
        // Bogie frame
        glPushMatrix(); glTranslatef(0,-0.05f,bz); glScalef(2.0f,0.30f,2.0f); glutSolidCube(1); glPopMatrix();
        // Axles
        for (int ax=-1; ax<=1; ax+=2) {
            glColor3f(0.3f,0.3f,0.35f);
            glPushMatrix();
            glTranslatef(-0.9f,-0.35f,bz+ax*0.7f);
            glRotatef(90,0,0,1);
            drawCylinder(0.10f,1.8f,8);
            glPopMatrix();
        }
        // Wheels
        glColor3f(0.22f,0.22f,0.25f);
        for (int wx2=-1; wx2<=1; wx2+=2) {
            for (int ax=-1; ax<=1; ax+=2) {
                glPushMatrix();
                glTranslatef(wx2*0.9f,-0.45f,bz+ax*0.7f);
                glRotatef(90,0,0,1);
                drawCylinder(0.35f,0.22f,12);
                glPopMatrix();
                // Wheel rim
                glColor3f(0.5f,0.5f,0.55f);
                glPushMatrix();
                glTranslatef(wx2*0.9f,-0.45f,bz+ax*0.7f);
                glRotatef(90,0,0,1);
                drawCylinder(0.38f,0.05f,12);
                glPopMatrix();
                glColor3f(0.22f,0.22f,0.25f);
            }
        }
    }

    // Roof AC unit
    glColor3f(0.55f,0.55f,0.6f);
    glPushMatrix(); glTranslatef(0,4.25f,0); glScalef(1.0f,0.4f,2.5f); glutSolidCube(1); glPopMatrix();

    // Coupler at rear
    glColor3f(0.4f,0.4f,0.45f);
    glPushMatrix(); glTranslatef(0,1.0f,-len*0.5f-0.25f); glScalef(0.45f,0.45f,0.5f); glutSolidCube(1); glPopMatrix();

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
            case 0: // BARRIER - fits within 1 lane (width ~2.0)
                glColor3f(0.95f,0.1f,0.1f);
                glPushMatrix(); glTranslatef(0,1.1f,0); glScalef(2.0f,2.2f,0.5f); glutSolidCube(1); glPopMatrix();
                glColor3f(1,1,1);
                for (int s=0;s<3;s++) {
                    glPushMatrix(); glTranslatef(-0.6f+s*0.6f,1.1f,0.27f); glScalef(0.22f,2.2f,0.05f); glutSolidCube(1); glPopMatrix();
                }
                // Legs
                glColor3f(0.6f,0.1f,0.1f);
                for (int l=-1;l<=1;l+=2) {
                    glPushMatrix(); glTranslatef(l*0.8f,0.55f,-0.2f); glScalef(0.18f,1.1f,0.5f); glutSolidCube(1); glPopMatrix();
                }
                // Warning light
                glDisable(GL_LIGHTING);
                glColor4f(1.0f,0.9f,0.0f, 0.9f);
                glPushMatrix(); glTranslatef(0,2.3f,0); drawSphere(0.2f,8,6); glPopMatrix();
                glEnable(GL_LIGHTING);
                break;

            case 1: // TRAIN
                glPopMatrix();
                drawTrainObstacle(ox, oz, o.color, o.length);
                continue;

            case 2: // LOW BEAM (duck under) - fits within 1 lane (width ~2.0)
                glColor3f(0.75f,0.55f,0.08f);
                for (int p=-1;p<=1;p+=2) {
                    glPushMatrix(); glTranslatef(p*0.85f,1.15f,0); glScalef(0.22f,2.3f,0.22f); glutSolidCube(1); glPopMatrix();
                }
                glColor3f(1.0f,0.25f,0.05f);
                glPushMatrix(); glTranslatef(0,2.3f,0); glScalef(2.0f,0.28f,0.28f); glutSolidCube(1); glPopMatrix();
                // Danger stripes on beam
                glDisable(GL_LIGHTING);
                for (int st=0;st<4;st++) {
                    glColor4f(st%2==0?1.0f:0.1f, st%2==0?0.85f:0.1f, 0.0f, 1.0f);
                    glBegin(GL_QUADS);
                    float sx = -0.85f + st*0.45f;
                    glVertex3f(sx,      2.16f, 0.15f);
                    glVertex3f(sx+0.32f,2.16f, 0.15f);
                    glVertex3f(sx+0.32f,2.44f, 0.15f);
                    glVertex3f(sx,      2.44f, 0.15f);
                    glEnd();
                }
                glEnable(GL_LIGHTING);
                break;

            case 3: // CONES ROW - small cones across 1 lane
                for (int cn=-1;cn<=1;cn++) {
                    glColor3f(1.0f,0.45f,0.0f);
                    glPushMatrix();
                    glTranslatef(cn*0.7f, 0.0f, cn*0.3f);
                    // Cone body
                    glPushMatrix(); glTranslatef(0,0.5f,0); glScalef(0.35f,1.0f,0.35f); glutSolidCube(1); glPopMatrix();
                    // Cone base
                    glColor3f(0.85f,0.85f,0.85f);
                    glPushMatrix(); glTranslatef(0,0.05f,0); glScalef(0.55f,0.12f,0.55f); glutSolidCube(1); glPopMatrix();
                    // Reflective stripe
                    glColor3f(1.0f,1.0f,1.0f);
                    glPushMatrix(); glTranslatef(0,0.58f,0); glScalef(0.37f,0.12f,0.37f); glutSolidCube(1); glPopMatrix();
                    glPopMatrix();
                }
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
        glRotatef(c.rot, 0,1,0);

        // Coin body
        glColor3f(1.0f, 0.82f, 0.08f);
        glPushMatrix(); glScalef(0.58f,0.58f,0.12f); glutSolidCube(1); glPopMatrix();

        // Inner ring
        glColor3f(0.95f,0.75f,0.05f);
        glPushMatrix(); glScalef(0.38f,0.38f,0.14f); glutSolidCube(1); glPopMatrix();

        // Highlight
        glDisable(GL_LIGHTING);
        glColor4f(1.0f,1.0f,0.7f,0.7f);
        glPushMatrix(); glTranslatef(0.08f,0.08f,0.07f); drawSphere(0.10f,6,4); glPopMatrix();
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }
}

// ============================================================
//  CAMERA SETUP
// ============================================================
void setupCamera() {
    float px = playerX;
    float py = GROUND_Y + playerY + 1.2f;
    float pz = 5.5f;

    switch (cameraMode) {
        case CAM_BACK:
            // Third-person behind
            gluLookAt(
                px*0.45f, 6.5f, 15.5f,
                px*0.25f, 0.5f, -5.0f,
                0, 1, 0
            );
            break;

        case CAM_SIDE:
            // Side camera (left side)
            gluLookAt(
                18.0f, 4.5f, pz,
                px,    py-0.5f, pz,
                0, 1, 0
            );
            break;

        case CAM_FIRST:
            // First-person (from player head)
            gluLookAt(
                px, py + 0.8f*(isSliding?0.3f:1.0f), pz,
                px*0.85f, py - 0.5f, pz - 18.0f,
                0, 1, 0
            );
            break;
    }
}

const char* cameraName() {
    switch(cameraMode) {
        case CAM_BACK:  return "Back";
        case CAM_SIDE:  return "Side";
        case CAM_FIRST: return "First-Person";
    }
    return "";
}

// ============================================================
//  DRAW HUD
// ============================================================
void drawHUD() {
    begin2D();

    // Top bar
    drawPanel(0, 565, WINDOW_W, WINDOW_H, 0.0f,0.0f,0.0f,0.55f, false);

    char buf[128];
    // Username
    glColor3f(0.65f,0.85f,1.0f);
    snprintf(buf,sizeof(buf),"Player: %s", username);
    drawText(16, 575, buf, GLUT_BITMAP_HELVETICA_12);

    // Score
    glColor3f(1.0f,0.85f,0.1f);
    snprintf(buf,sizeof(buf),"Score: %d", score);
    drawText(180, 575, buf, GLUT_BITMAP_HELVETICA_18);

    // Coins
    glColor3f(1.0f,1.0f,0.35f);
    snprintf(buf,sizeof(buf),"Coins: %d", coinCount);
    drawText(360, 575, buf, GLUT_BITMAP_HELVETICA_18);

    // Speed
    glColor3f(0.45f,1.0f,0.55f);
    snprintf(buf,sizeof(buf),"Speed: x%.1f", gameSpeed/0.55f);
    drawText(540, 575, buf, GLUT_BITMAP_HELVETICA_12);

    // Camera
    glColor3f(0.8f,0.6f,1.0f);
    snprintf(buf,sizeof(buf),"Cam: %s [C]", cameraName());
    drawText(680, 575, buf, GLUT_BITMAP_HELVETICA_12);

    // Slide/jump notifier
    if (isSliding) {
        glColor4f(0.2f,0.9f,1.0f,0.9f);
        drawTextCentered(48, "SLIDING!", GLUT_BITMAP_HELVETICA_18);
    }
    if (isJumping) {
        glColor4f(0.5f,1.0f,0.5f,0.9f);
        drawTextCentered(48, "JUMPING!", GLUT_BITMAP_HELVETICA_12);
    }

    end2D();
}

// ============================================================
//  DRAW INTRO
// ============================================================
void drawIntro() {
    begin2D();

    // Background gradient (simulate)
    glColor4f(0.04f,0.04f,0.12f,1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_W,0);
    glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H);
    glEnd();

    // Pulsing title
    float pulse = 0.85f + 0.15f*sinf(introTimer*3.0f);

    // Shadow
    glColor4f(0.0f,0.0f,0.0f,0.7f);
    drawTextCentered(342, "SUBWAY RUNNER", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0f*pulse, 0.82f*pulse, 0.05f*pulse);
    drawTextCentered(345, "SUBWAY RUNNER", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.7f,0.7f,0.75f);
    drawTextCentered(295, "An OpenGL Endless Runner", GLUT_BITMAP_HELVETICA_12);

    // Loading bar
    float prog = clamp(introTimer/4.0f, 0.0f, 1.0f);
    drawPanel(200, 228, 700, 248, 0.1f,0.1f,0.15f,0.9f, true);
    glColor4f(0.2f,0.65f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glVertex2f(202,230); glVertex2f(202+496*prog,230);
    glVertex2f(202+496*prog,246); glVertex2f(202,246);
    glEnd();

    if (introTimer >= 4.0f) {
        glColor4f(0.85f,0.85f,0.9f, 0.5f+0.5f*sinf(introTimer*4.0f));
        drawTextCentered(195, "Press ENTER to continue", GLUT_BITMAP_HELVETICA_12);
    }

    // Version
    glColor3f(0.35f,0.35f,0.4f);
    drawText(8, 10, "v2.0 Enhanced Edition", GLUT_BITMAP_HELVETICA_12);

    end2D();
}

// ============================================================
//  DRAW LOGIN
// ============================================================
void drawLogin() {
    begin2D();

    // Dark overlay
    glColor4f(0.04f,0.04f,0.12f,1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_W,0);
    glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H);
    glEnd();

    // Panel
    drawPanel(250,200,650,430, 0.06f,0.06f,0.18f,0.92f);

    // Title
    glColor3f(1.0f,0.82f,0.08f);
    drawTextCentered(400, "SUBWAY RUNNER", GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.7f,0.75f,0.85f);
    drawTextCentered(368, "Enter your username to continue", GLUT_BITMAP_HELVETICA_12);

    // Input box
    bool blink = (int)(introTimer*2)%2==0;
    drawPanel(285,310,615,345, 0.1f,0.1f,0.22f,0.95f);
    glColor3f(0.4f,0.75f,1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(285,310); glVertex2f(615,310);
    glVertex2f(615,345); glVertex2f(285,345);
    glEnd();

    char displayName[USERNAME_MAX+2];
    snprintf(displayName, sizeof(displayName), "%s%s", username, blink?"|":"");
    glColor3f(1.0f,1.0f,1.0f);
    drawText(295, 322, displayName, GLUT_BITMAP_HELVETICA_18);

    // Label
    glColor3f(0.6f,0.7f,0.85f);
    drawText(295, 358, "Username:", GLUT_BITMAP_HELVETICA_12);

    // Enter button
    drawMenuButton(450, 278, 120, 32, "[ ENTER ]", true, GLUT_BITMAP_HELVETICA_12);

    // Error message
    if (loginError) {
        glColor3f(1.0f, 0.3f, 0.3f);
        drawTextCentered(248, loginMsg, GLUT_BITMAP_HELVETICA_12);
    }

    // Hint
    glColor3f(0.4f,0.45f,0.55f);
    drawTextCentered(218, "Press ENTER to confirm", GLUT_BITMAP_HELVETICA_12);

    end2D();
}

// ============================================================
//  DRAW MENU
// ============================================================
void drawMenu() {
    begin2D();

    // Background
    glColor4f(0.04f,0.04f,0.12f,0.92f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(WINDOW_W,0);
    glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H);
    glEnd();

    // Logo panel
    drawPanel(220,410,680,490, 0.05f,0.05f,0.18f,0.92f);
    glColor3f(1.0f,0.82f,0.08f);
    drawTextCentered(455, "SUBWAY RUNNER", GLUT_BITMAP_TIMES_ROMAN_24);

    char greet[64];
    snprintf(greet,sizeof(greet),"Welcome, %s!", username);
    glColor3f(0.65f,0.8f,1.0f);
    drawTextCentered(428, greet, GLUT_BITMAP_HELVETICA_12);

    if (!showControls) {
        // Menu buttons
        drawPanel(260,120,640,400, 0.06f,0.06f,0.18f,0.88f);

        float btnY[MENU_COUNT] = {355, 295, 235, 160};
        for (int i=0; i<MENU_COUNT; i++) {
            drawMenuButton(450, btnY[i], 260, 44, menuItems[i],
                           i==menuSelection, GLUT_BITMAP_HELVETICA_18);
        }

        glColor3f(0.4f,0.45f,0.55f);
        drawTextCentered(128, "UP/DOWN to navigate  |  ENTER to select", GLUT_BITMAP_HELVETICA_12);

        // Camera mode hint
        if (menuSelection==1) {
            glColor3f(0.8f,0.6f,1.0f);
            char camBuf[48];
            snprintf(camBuf,sizeof(camBuf),"Current: %s   (ENTER to cycle)", cameraName());
            drawTextCentered(108, camBuf, GLUT_BITMAP_HELVETICA_12);
        }
    } else {
        // Controls panel
        drawPanel(200,80,700,400, 0.06f,0.06f,0.18f,0.92f);

        glColor3f(1.0f,0.85f,0.1f);
        drawTextCentered(378, "CONTROLS", GLUT_BITMAP_HELVETICA_18);

        struct CtrlEntry { const char* key; const char* action; };
        CtrlEntry ctrls[] = {
            {"LEFT / A",     "Move lane left"},
            {"RIGHT / D",    "Move lane right"},
            {"SPACE",        "Jump"},
            {"DOWN / S",     "Slide"},
            {"C",            "Toggle camera"},
            {"R",            "Restart (Game Over)"},
            {"ESC",          "Quit"},
        };
        int nc = 7;
        for (int i=0; i<nc; i++) {
            float cy2 = 345.0f - i*38.0f;
            glColor3f(0.4f,0.75f,1.0f);
            drawText(230, cy2, ctrls[i].key, GLUT_BITMAP_HELVETICA_12);
            glColor3f(0.85f,0.85f,0.9f);
            drawText(430, cy2, ctrls[i].action, GLUT_BITMAP_HELVETICA_12);
        }

        glColor3f(0.5f,1.0f,0.5f);
        drawTextCentered(98, "Press BACKSPACE to go back", GLUT_BITMAP_HELVETICA_12);
    }

    end2D();
}

// ============================================================
//  DRAW GAMEOVER
// ============================================================
void drawGameOver() {
    begin2D();
    drawPanel(160,120,740,490, 0.08f,0.0f,0.0f,0.92f);

    // Title
    glColor3f(1.0f,0.15f,0.15f);
    drawTextCentered(450, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

    // Player name
    char buf[128];
    glColor3f(0.65f,0.85f,1.0f);
    snprintf(buf,sizeof(buf),"Player: %s", username);
    drawTextCentered(412, buf, GLUT_BITMAP_HELVETICA_18);

    // Divider
    glColor4f(1,1,1,0.2f);
    glBegin(GL_LINES);
    glVertex2f(200,402); glVertex2f(700,402);
    glEnd();

    // Stats
    glColor3f(1.0f,0.85f,0.1f);
    snprintf(buf,sizeof(buf),"Final Score:  %d", score);
    drawTextCentered(370, buf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(1.0f,1.0f,0.35f);
    snprintf(buf,sizeof(buf),"Coins Collected:  %d", coinCount);
    drawTextCentered(335, buf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.55f,1.0f,0.55f);
    snprintf(buf,sizeof(buf),"Top Speed:  x%.2f", topSpeed/0.55f);
    drawTextCentered(298, buf, GLUT_BITMAP_HELVETICA_12);

    // Divider
    glColor4f(1,1,1,0.2f);
    glBegin(GL_LINES);
    glVertex2f(200,282); glVertex2f(700,282);
    glEnd();

    // Buttons
    float blink = 0.65f + 0.35f*sinf(introTimer*3.0f);
    glColor3f(0.3f*blink, 1.0f*blink, 0.35f*blink);
    drawTextCentered(240, "Press  R  to Restart", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.6f,0.6f,0.65f);
    drawTextCentered(188, "Press  M  to go to Main Menu", GLUT_BITMAP_HELVETICA_12);

    glColor3f(0.45f,0.45f,0.5f);
    drawTextCentered(158, "Press  ESC  to Quit", GLUT_BITMAP_HELVETICA_12);

    end2D();
}

// ============================================================
//  DISPLAY
// ============================================================
void display() {
    float sr,sg,sb;
    hsvToRgb(fmod(envHue+0.55f,1.0f), 0.4f, 0.65f, sr,sg,sb);

    if (gameState==INTRO || gameState==LOGIN || gameState==MENU) {
        glClearColor(0.04f,0.04f,0.12f,1.0f);
    } else {
        glClearColor(sr,sg,sb,1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (gameState == INTRO) {
        drawIntro();
        glutSwapBuffers();
        return;
    }
    if (gameState == LOGIN) {
        drawLogin();
        glutSwapBuffers();
        return;
    }
    if (gameState == MENU) {
        // Draw game world as backdrop
        gluLookAt(0,5.5f,14.0f, 0,0.5f,-5.0f, 0,1,0);
        enableFog();
        drawSky();
        drawClouds();
        drawBuildings();
        drawStreetLamps();
        drawRealisticGround();
        glDisable(GL_FOG);
        drawMenu();
        glutSwapBuffers();
        return;
    }

    // PLAYING / GAMEOVER
    enableFog();
    drawSky();
    drawClouds();
    setupCamera();
    drawBuildings();
    drawStreetLamps();
    drawRealisticGround();
    drawCoins();
    drawObstacles();
    if (cameraMode != CAM_FIRST) drawPlayer();
    glDisable(GL_FOG);

    drawHUD();
    if (gameState == GAMEOVER) drawGameOver();

    glutSwapBuffers();
}

// ============================================================
//  COLLISION
// ============================================================
bool checkObstacleCollision(Obstacle& o) {
    float px = playerX;
    float pz = 5.5f;
    float ox = laneX(o.lane);
    float oz = o.z;

    float dz = fabs(pz - oz);

    // Long train spans all 3 lanes check
    if (o.type == 1 || o.type == 3) {
        float dx = fabs(px - ox);
        if (dz > o.length*0.5f + 0.5f) return false;
        if (dx > 1.1f) return false;
        if (playerY > 3.5f) return false;
        return true;
    }

    float dx = fabs(px - ox);
    if (dx > 1.0f || dz > 2.0f) return false;

    // type 2: low beam
    if (o.type == 2) {
        if (isSliding) return false;
        if (playerY >= 1.0f) return false;
        return true;
    }

    if (playerY > 2.8f) return false;
    return true;
}

// ============================================================
//  UPDATE
// ============================================================
void update(int value) {
    introTimer += 0.013f;

    if (gameState == PLAYING) {
        // Speed ramp
        speedTimer += 0.013f;
        gameSpeed   = 0.55f + speedTimer*0.011f;
        if (gameSpeed > 1.8f) gameSpeed = 1.8f;
        if (gameSpeed > topSpeed) topSpeed = gameSpeed;

        // Environment
        envHue += 0.00012f;
        if (envHue > 1.0f) envHue -= 1.0f;

        // Animation
        legAngle += gameSpeed * 6.5f;
        armAngle  = legAngle;

        // Squash
        if (squashTimer > 0) {
            squashTimer -= 0.05f;
            squashY = lerp(1.0f, 0.5f, squashTimer);
        } else squashY = 1.0f;

        // Lane lerp
        float targetX = laneX(playerLane);
        playerX = lerp(playerX, targetX, 0.20f);

        // Jump
        if (isJumping) {
            playerY    += playerVelY;
            playerVelY -= 0.011f;
            if (playerY <= 0.0f) {
                playerY     = 0.0f;
                isJumping   = false;
                playerVelY  = 0.0f;
                squashY     = 0.5f;
                squashTimer = 1.0f;
            }
        }

        // Slide
        if (isSliding) {
            slideTimer -= 0.022f;
            if (slideTimer <= 0) isSliding = false;
        }

        // World
        worldOffset += gameSpeed;

        // Score
        scoreTimer += gameSpeed;
        if (scoreTimer >= 10.0f) { score++; scoreTimer = 0.0f; }

        // Obstacles
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            obstacles[i].z += gameSpeed;
            if (obstacles[i].z > 14.0f) {
                // Ensure minimum gap before respawning
                float minZ = -55.0f;
                for (int j=0; j<MAX_OBSTACLES; j++) {
                    if (j!=i && obstacles[j].z < 0 && obstacles[j].z > minZ)
                        minZ = obstacles[j].z - 18.0f;
                }
                obstacles[i].z    = minZ - (rand()%15);
                obstacles[i].lane = (rand()%3)-1;
                obstacles[i].type = rand()%4;
                obstacles[i].active = true;
                obstacles[i].length = 5.5f + (rand()%5);
                obstacles[i].color  = rand()%4;
            }
            if (obstacles[i].active && checkObstacleCollision(obstacles[i])) {
                gameState = GAMEOVER;
            }
        }

        // Coins
        for (int i = 0; i < MAX_COINS; i++) {
            if (!coins[i].active) continue;
            coins[i].z   += gameSpeed;
            coins[i].rot += 5.0f;
            if (coins[i].z > 14.0f) {
                coins[i].z      = -45.0f - (rand()%30);
                coins[i].x      = laneX((rand()%3)-1);
                coins[i].y      = GROUND_Y + 1.5f;
                coins[i].active = true;
            }
            float dx = fabs(playerX-coins[i].x);
            float dz = fabs(5.5f-coins[i].z);
            float dy = fabs((GROUND_Y+playerY+1.2f)-coins[i].y);
            if (dx<1.3f && dz<1.6f && dy<1.6f) {
                coins[i].active = false;
                coinCount++;
                score += 5;
            }
        }

        // Clouds
        for (int i=0; i<MAX_CLOUDS; i++) {
            clouds[i].x += clouds[i].speed;
            if (clouds[i].x > 55) clouds[i].x = -55;
        }

        // Lamps recycle
        for (int i=0; i<MAX_LAMPS; i++) {
            float lz = lamps[i].z + fmod(worldOffset, 16.0f);
            if (lz > 16.0f) lamps[i].z -= MAX_LAMPS*16.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(13, update, 0);
}

// ============================================================
//  INPUT: SPECIAL KEYS
// ============================================================
void keySpecial(int key, int x, int y) {
    if (gameState == MENU && !showControls) {
        if (key == GLUT_KEY_UP)   menuSelection = (menuSelection-1+MENU_COUNT)%MENU_COUNT;
        if (key == GLUT_KEY_DOWN) menuSelection = (menuSelection+1)%MENU_COUNT;
        return;
    }
    if (gameState != PLAYING) return;
    if (key == GLUT_KEY_LEFT  && playerLane > -1) playerLane--;
    if (key == GLUT_KEY_RIGHT && playerLane <  1) playerLane++;
    if (key == GLUT_KEY_DOWN && !isJumping) { isSliding=true; slideTimer=1.0f; }
}

// ============================================================
//  INPUT: NORMAL KEYS
// ============================================================
void keyNormal(unsigned char key, int x, int y) {
    if (key == 27) exit(0); // ESC always quits

    // INTRO state
    if (gameState == INTRO) {
        if (key == '\r' || key == '\n') {
            if (introTimer >= 4.0f) gameState = LOGIN;
        }
        return;
    }

    // LOGIN state
    if (gameState == LOGIN) {
        if ((key == '\r' || key == '\n')) {
            if (usernameLen == 0) {
                loginError = true;
                snprintf(loginMsg,sizeof(loginMsg),"Username cannot be empty!");
            } else {
                loginError = false;
                gameState  = MENU;
            }
            return;
        }
        if (key == 8 || key == 127) { // backspace/delete
            if (usernameLen > 0) { usernameLen--; username[usernameLen] = '\0'; }
            return;
        }
        if (key >= 32 && key < 127 && usernameLen < USERNAME_MAX-1) {
            username[usernameLen++] = key;
            username[usernameLen]   = '\0';
            loginError = false;
        }
        return;
    }

    // MENU state
    if (gameState == MENU) {
        if (showControls) {
            if (key == 8 || key == 127) showControls = false;
            return;
        }
        if (key == '\r' || key == '\n') {
            switch (menuSelection) {
                case 0: initGame(); gameState = PLAYING; break;
                case 1: // Cycle camera
                    cameraMode = (CameraMode)((cameraMode+1)%3);
                    break;
                case 2: showControls = true; break;
                case 3: exit(0); break;
            }
        }
        return;
    }

    // GAMEOVER state
    if (gameState == GAMEOVER) {
        if (key == 'r' || key == 'R') { initGame(); gameState = PLAYING; }
        if (key == 'm' || key == 'M') { gameState = MENU; menuSelection = 0; showControls = false; }
        return;
    }

    // PLAYING state
    if (gameState == PLAYING) {
        if (key == ' ' && !isJumping && !isSliding) {
            isJumping  = true;
            playerVelY = 0.30f;
        }
        if ((key=='s'||key=='S') && !isJumping) { isSliding=true; slideTimer=1.0f; }
        if (key=='a'||key=='A') { if (playerLane>-1) playerLane--; }
        if (key=='d'||key=='D') { if (playerLane< 1) playerLane++; }
        if (key=='c'||key=='C') cameraMode=(CameraMode)((cameraMode+1)%3);
    }
}

// ============================================================
//  RESHAPE
// ============================================================
void reshape(int w, int h) {
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w/h, 0.3, 350.0);
    glMatrixMode(GL_MODELVIEW);
}

// ============================================================
//  INIT GL
// ============================================================
void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Main sun light
    GLfloat lPos[]  = { 8.0f, 25.0f, 8.0f, 1.0f };
    GLfloat lAmb[]  = { 0.38f,0.38f,0.42f,1.0f };
    GLfloat lDiff[] = { 1.0f, 0.96f, 0.88f,1.0f };
    GLfloat lSpec[] = { 0.65f,0.65f,0.65f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lSpec);

    // Fill light from below
    glEnable(GL_LIGHT1);
    GLfloat l1Pos[] = { 0.0f, -5.0f, 10.0f, 1.0f };
    GLfloat l1Amb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat l1Diff[]= { 0.18f,0.2f,0.28f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, l1Pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  l1Amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  l1Diff);

    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)WINDOW_W/WINDOW_H, 0.3, 350.0);
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
    glutInitWindowPosition(80, 60);
    glutCreateWindow("Subway Runner - Enhanced Edition");

    initGL();
    initGame();

    // Reset for intro
    gameState = INTRO;
    introTimer = 0.0f;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keySpecial);
    glutKeyboardFunc(keyNormal);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
