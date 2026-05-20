#include "../header/Init.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include "../header/Texture.h"
#include <GL/glut.h>
#include <stdlib.h>

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
        buildings[i].z = -12.0f - (i/2)*10.0f;
        buildings[i].h = h;
        buildings[i].w = w;
        buildings[i].side = side;
        buildings[i].r = r2;
        buildings[i].g = g2;
        buildings[i].b = b2;
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
        clouds[i].x = -30.0f + (float)(rand()%60);
        clouds[i].y = 14.0f + (float)(rand()%8);
        clouds[i].z = -20.0f - (float)(rand()%40);
        clouds[i].speed = 0.015f + (float)(rand()%3)*0.008f;
        clouds[i].scale = 1.5f + (float)(rand()%3);
    }
}

void initGL() {
    buildingTexture = loadTexture("texture/undip1.bmp");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lPos[]  = { 8.0f, 25.0f, 8.0f, 1.0f };
    GLfloat lAmb[]  = { 0.38f,0.38f,0.42f,1.0f };
    GLfloat lDiff[] = { 1.0f, 0.96f, 0.88f,1.0f };
    GLfloat lSpec[] = { 0.65f,0.65f,0.65f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lSpec);

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
