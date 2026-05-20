#include "../header/Obstacles.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include "../header/Player.h"
#include <GL/glut.h>

void drawHorseObstacle(float ox, float oz, int colorVar, float len) {
    float horseColors[4][3] = {
        {0.55f, 0.27f, 0.07f},
        {0.15f, 0.15f, 0.15f},
        {0.8f,  0.8f,  0.8f },
        {0.6f,  0.45f, 0.25f}
    };
    float r = horseColors[colorVar][0];
    float g = horseColors[colorVar][1];
    float b = horseColors[colorVar][2];

    glPushMatrix();
    glTranslatef(ox, GROUND_Y + 1.2f, oz);
    glRotatef(-90.0f, 0, 1, 0);

    glColor3f(r, g, b);
    glPushMatrix(); glScalef(2.8f, 1.4f, 1.2f); glutSolidCube(1); glPopMatrix();

    glPushMatrix(); glTranslatef(1.2f, 0.8f, 0); glRotatef(-35, 0, 0, 1); glScalef(0.6f, 1.4f, 0.6f); glutSolidCube(1); glPopMatrix();

    glPushMatrix(); glTranslatef(1.8f, 1.5f, 0); glScalef(1.0f, 0.7f, 0.7f); glutSolidCube(1); glPopMatrix();

    for(int e=-1; e<=1; e+=2){ glPushMatrix(); glTranslatef(2.1f, 2.0f, e*0.18f); glScalef(0.15f, 0.4f, 0.15f); glutSolidCube(1); glPopMatrix(); }

    for(int lx=-1; lx<=1; lx+=2){ for(int lz=-1; lz<=1; lz+=2){ glPushMatrix(); glTranslatef(lx*0.8f, -1.2f, lz*0.35f); glScalef(0.35f, 2.0f, 0.35f); glutSolidCube(1); glPopMatrix(); }}

    glColor3f(r*0.5f, g*0.5f, b*0.5f);
    glPushMatrix(); glTranslatef(-1.5f, 0.5f, 0); glRotatef(40, 0, 0, 1); glScalef(0.25f, 1.3f, 0.25f); glutSolidCube(1); glPopMatrix();

    glColor3f(0,0,0);
    for(int e=-1; e<=1; e+=2){ glPushMatrix(); glTranslatef(2.25f, 1.6f, e*0.2f); drawSphere(0.07f, 6, 6); glPopMatrix(); }

    glPopMatrix();
}

void drawObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        Obstacle& o = obstacles[i];
        float ox = laneX(o.lane);
        float oz = o.z;

        glPushMatrix();
        glTranslatef(ox, GROUND_Y, oz);

        switch (o.type) {
            case 0:
                glColor3f(0.95f,0.1f,0.1f);
                glPushMatrix(); glTranslatef(0,1.1f,0); glScalef(2.0f,2.2f,0.5f); glutSolidCube(1); glPopMatrix();
                glColor3f(1,1,1);
                for (int s=0;s<3;s++) { glPushMatrix(); glTranslatef(-0.6f+s*0.6f,1.1f,0.27f); glScalef(0.22f,2.2f,0.05f); glutSolidCube(1); glPopMatrix(); }
                glColor3f(0.6f,0.1f,0.1f);
                for (int l=-1;l<=1;l+=2) { glPushMatrix(); glTranslatef(l*0.8f,0.55f,-0.2f); glScalef(0.18f,1.1f,0.5f); glutSolidCube(1); glPopMatrix(); }
                glDisable(GL_LIGHTING);
                glColor4f(1.0f,0.9f,0.0f, 0.9f);
                glPushMatrix(); glTranslatef(0,2.3f,0); drawSphere(0.2f,8,6); glPopMatrix();
                glEnable(GL_LIGHTING);
                break;
            case 1:
                glPopMatrix();
                drawHorseObstacle(ox, oz, o.color, o.length);
                continue;
            case 2:
                glColor3f(0.75f,0.55f,0.08f);
                for (int p=-1;p<=1;p+=2) { glPushMatrix(); glTranslatef(p*0.85f,1.15f,0); glScalef(0.22f,2.3f,0.22f); glutSolidCube(1); glPopMatrix(); }
                glColor3f(1.0f,0.25f,0.05f);
                glPushMatrix(); glTranslatef(0,2.3f,0); glScalef(2.0f,0.28f,0.28f); glutSolidCube(1); glPopMatrix();
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
            case 3:
                for (int cn=-1;cn<=1;cn++) {
                    glColor3f(1.0f,0.45f,0.0f);
                    glPushMatrix(); glTranslatef(cn*0.7f, 0.0f, cn*0.3f);
                    glPushMatrix(); glTranslatef(0,0.5f,0); glScalef(0.35f,1.0f,0.35f); glutSolidCube(1); glPopMatrix();
                    glColor3f(0.85f,0.85f,0.85f);
                    glPushMatrix(); glTranslatef(0,0.05f,0); glScalef(0.55f,0.12f,0.55f); glutSolidCube(1); glPopMatrix();
                    glColor3f(1.0f,1.0f,1.0f);
                    glPushMatrix(); glTranslatef(0,0.58f,0); glScalef(0.37f,0.12f,0.37f); glutSolidCube(1); glPopMatrix();
                    glPopMatrix();
                }
                break;
        }
        glPopMatrix();
    }
}
