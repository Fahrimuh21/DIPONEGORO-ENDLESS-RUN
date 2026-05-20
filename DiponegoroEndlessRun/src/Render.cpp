#include "../header/Render.h"
#include "../header/Globals.h"
#include "../header/UI.h"
#include "../header/Environment.h"
#include "../header/Coins.h"
#include "../header/Obstacles.h"
#include "../header/Player.h"
#include "../header/Camera.h"
#include "../header/Utility.h"
#include <GL/glut.h>
#include <math.h>

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

void drawRealisticGround() {
    const float TL = -3.75f; const float TR =  3.75f; const float D1 = -1.25f; const float D2 =  1.25f;
    for (int i = 0; i < 28; i++) {
        float z  = 15.0f - i*10.0f + fmod(worldOffset, 10.0f);
        float z2 = z - 10.0f;

        glDisable(GL_LIGHTING);
        float s = (i%2==0) ? 0.33f : 0.28f;
        glColor3f(s, s-0.02f, s+0.04f);
        glBegin(GL_QUADS); glVertex3f(TL,GROUND_Y,z); glVertex3f(D1,GROUND_Y,z); glVertex3f(D1,GROUND_Y,z2); glVertex3f(TL,GROUND_Y,z2); glEnd();
        s = (i%2==0) ? 0.37f : 0.31f;
        glColor3f(s, s-0.02f, s+0.04f); glBegin(GL_QUADS); glVertex3f(D1,GROUND_Y,z); glVertex3f(D2,GROUND_Y,z); glVertex3f(D2,GROUND_Y,z2); glVertex3f(D1,GROUND_Y,z2); glEnd();
        s = (i%2==0) ? 0.33f : 0.28f;
        glColor3f(s, s-0.02f, s+0.04f); glBegin(GL_QUADS); glVertex3f(D2,GROUND_Y,z); glVertex3f(TR,GROUND_Y,z); glVertex3f(TR,GROUND_Y,z2); glVertex3f(D2,GROUND_Y,z2); glEnd();

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

        glColor3f(0.95f, 0.80f, 0.0f);
        for (int s2=0; s2<2; s2++) { float bx = (s2==0) ? TL : TR - 0.12f; glBegin(GL_QUADS); glVertex3f(bx, GROUND_Y+0.02f, z); glVertex3f(bx+0.12f, GROUND_Y+0.02f, z); glVertex3f(bx+0.12f, GROUND_Y+0.02f, z2); glVertex3f(bx, GROUND_Y+0.02f, z2); glEnd(); }

        float railX[2] = { TL + 0.35f, TR - 0.57f };
        for (int r=0; r<2; r++) {
            float rx = railX[r]; glColor3f(0.72f, 0.68f, 0.62f); glBegin(GL_QUADS); glVertex3f(rx, GROUND_Y+0.04f, z); glVertex3f(rx+0.22f, GROUND_Y+0.04f, z); glVertex3f(rx+0.22f, GROUND_Y+0.04f, z2); glVertex3f(rx, GROUND_Y+0.04f, z2); glEnd();
            glColor3f(0.88f, 0.85f, 0.78f); glBegin(GL_QUADS); glVertex3f(rx+0.04f, GROUND_Y+0.06f, z); glVertex3f(rx+0.18f, GROUND_Y+0.06f, z); glVertex3f(rx+0.18f, GROUND_Y+0.06f, z2); glVertex3f(rx+0.04f, GROUND_Y+0.06f, z2); glEnd();
        }

        for (int j=0; j<5; j++) {
            float sz = z - j*2.0f - 0.5f;
            glColor3f(0.42f, 0.27f, 0.14f); glBegin(GL_QUADS); glVertex3f(TL-0.2f, GROUND_Y+0.02f, sz); glVertex3f(TR+0.2f, GROUND_Y+0.02f, sz); glVertex3f(TR+0.2f, GROUND_Y+0.02f, sz-0.24f); glVertex3f(TL-0.2f, GROUND_Y+0.02f, sz-0.24f); glEnd();
            glColor3f(0.28f, 0.18f, 0.08f); glBegin(GL_QUADS); glVertex3f(TL-0.2f, GROUND_Y+0.01f, sz-0.24f); glVertex3f(TR+0.2f, GROUND_Y+0.01f, sz-0.24f); glVertex3f(TR+0.2f, GROUND_Y+0.01f, sz-0.32f); glVertex3f(TL-0.2f, GROUND_Y+0.01f, sz-0.32f); glEnd();
        }

        glEnable(GL_LIGHTING);

        glColor3f(0.68f, 0.68f, 0.72f); glBegin(GL_QUADS); glVertex3f(TL-12.0f,GROUND_Y,z); glVertex3f(TL,GROUND_Y,z); glVertex3f(TL,GROUND_Y,z2); glVertex3f(TL-12.0f,GROUND_Y,z2); glEnd();
        glBegin(GL_QUADS); glVertex3f(TR,GROUND_Y,z); glVertex3f(TR+12.0f,GROUND_Y,z); glVertex3f(TR+12.0f,GROUND_Y,z2); glVertex3f(TR,GROUND_Y,z2); glEnd();

        glDisable(GL_LIGHTING);
        glColor3f(0.95f, 0.85f, 0.08f); glBegin(GL_QUADS); glVertex3f(TL-0.22f,GROUND_Y+0.03f,z); glVertex3f(TL,GROUND_Y+0.03f,z); glVertex3f(TL,GROUND_Y+0.03f,z2); glVertex3f(TL-0.22f,GROUND_Y+0.03f,z2); glEnd();
        glBegin(GL_QUADS); glVertex3f(TR,GROUND_Y+0.03f,z); glVertex3f(TR+0.22f,GROUND_Y+0.03f,z); glVertex3f(TR+0.22f,GROUND_Y+0.03f,z2); glVertex3f(TR,GROUND_Y+0.03f,z2); glEnd();

        if (i == 0) {
            glColor4f(0,0,0,0.28f);
            float srad = isSliding ? 1.0f : 0.75f;
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(playerX, GROUND_Y+0.01f, 5.5f);
            for (int k=0;k<=18;k++) { float ang = k*(2*3.14159265f/18); glVertex3f(playerX+cosf(ang)*srad, GROUND_Y+0.01f, 5.5f+sinf(ang)*0.5f); }
            glEnd();
        }

        glEnable(GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);
    glColor3f(0.15f,0.15f,0.20f);
    glBegin(GL_QUADS); glVertex3f(-10,12,-200); glVertex3f(10,12,-200); glVertex3f(10,GROUND_Y,-200); glVertex3f(-10,GROUND_Y,-200); glEnd();
    glEnable(GL_LIGHTING);
}

void enableFog() {
    float sr,sg,sb; hsvToRgb(fmod(envHue+0.08f,1.0f),0.4f,0.75f,sr,sg,sb);
    GLfloat fogColor[4] = {sr,sg,sb,1.0f};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 40.0f);
    glFogf(GL_FOG_END,  130.0f);
    glEnable(GL_FOG);
}

void display() {
    float sr,sg,sb; hsvToRgb(fmod(envHue+0.55f,1.0f), 0.4f, 0.65f, sr,sg,sb);
    if (gameState==INTRO || gameState==LOGIN || gameState==MENU) glClearColor(0.04f,0.04f,0.12f,1.0f);
    else glClearColor(sr,sg,sb,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (gameState == INTRO) { drawIntro(); glutSwapBuffers(); return; }
    if (gameState == LOGIN) { drawLogin(); glutSwapBuffers(); return; }
    if (gameState == MENU) {
        gluLookAt(0,5.5f,14.0f, 0,0.5f,-5.0f, 0,1,0);
        enableFog(); drawSky(); drawClouds(); drawBuildings(); drawStreetLamps(); drawRealisticGround(); glDisable(GL_FOG); drawMenu(); glutSwapBuffers(); return;
    }

    enableFog(); drawSky(); drawClouds(); setupCamera(); drawBuildings(); drawStreetLamps(); drawRealisticGround(); drawCoins(); drawObstacles(); if (cameraMode != CAM_FIRST) drawPlayer(); glDisable(GL_FOG);
    drawHUD(); if (gameState == GAMEOVER) drawGameOver();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w/h, 0.3, 350.0);
    glMatrixMode(GL_MODELVIEW);
}
