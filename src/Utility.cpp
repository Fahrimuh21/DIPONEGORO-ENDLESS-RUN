#include "../header/Utility.h"
#include "../header/Constants.h"
#include <GL/glut.h>
#include <math.h>

float laneX(int l) {
    if (l == -1) return LANE_LEFT;
    if (l ==  1) return LANE_RIGHT;
    return LANE_MID;
}

float lerp(float a, float b, float t) { return a + (b - a) * t; }

float clampf(float v, float lo, float hi) {
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

void drawCylinder(float radius, float height, int slices) {
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

void drawSphere(float radius, int slices, int stacks) {
    GLUquadric* q = gluNewQuadric();
    gluSphere(q, radius, slices, stacks);
    gluDeleteQuadric(q);
}
