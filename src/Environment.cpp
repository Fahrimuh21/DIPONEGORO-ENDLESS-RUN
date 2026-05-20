#include "../header/Environment.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>

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

void drawStreetLamps() {
    for (int i = 0; i < MAX_LAMPS; i++) {
        float lz = lamps[i].z + fmod(worldOffset, 16.0f);
        float lx = lamps[i].side * 8.5f;

        if (lz > 16.0f) { lamps[i].z -= MAX_LAMPS * 16.0f; continue; }

        glColor3f(0.55f,0.55f,0.6f);
        glPushMatrix(); glTranslatef(lx, GROUND_Y, lz); drawCylinder(0.12f, 7.0f, 8); glPopMatrix();

        glColor3f(0.55f,0.55f,0.6f);
        glPushMatrix();
        float armDir = lamps[i].side * -1.0f;
        glTranslatef(lx + armDir*0.8f, GROUND_Y+6.5f, lz);
        glScalef(1.6f, 0.12f, 0.12f);
        glutSolidCube(1);
        glPopMatrix();

        glColor3f(1.0f, 0.92f, 0.6f);
        glPushMatrix(); glTranslatef(lx + armDir*1.5f, GROUND_Y+6.4f, lz); glScalef(0.6f, 0.22f, 0.35f); glutSolidCube(1); glPopMatrix();

        glDisable(GL_LIGHTING);
        glColor4f(1.0f, 0.95f, 0.5f, 0.18f);
        glPushMatrix(); glTranslatef(lx + armDir*1.5f, GROUND_Y+6.3f, lz); drawSphere(0.6f, 10, 8); glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}

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

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, buildingTexture);

        glPushMatrix();
        glTranslatef(bx, by, bz);
        glScalef(b.w, b.h, 5.5f);

        glBegin(GL_QUADS);
        // FRONT
        glTexCoord2f(0,0); glVertex3f(-0.5,-0.5, 0.5);
        glTexCoord2f(1,0); glVertex3f( 0.5,-0.5, 0.5);
        glTexCoord2f(1,1); glVertex3f( 0.5, 0.5, 0.5);
        glTexCoord2f(0,1); glVertex3f(-0.5, 0.5, 0.5);
        // BACK
        glTexCoord2f(0,0); glVertex3f(-0.5,-0.5,-0.5);
        glTexCoord2f(1,0); glVertex3f( 0.5,-0.5,-0.5);
        glTexCoord2f(1,1); glVertex3f( 0.5, 0.5,-0.5);
        glTexCoord2f(0,1); glVertex3f(-0.5, 0.5,-0.5);
        // LEFT
        glTexCoord2f(0,0); glVertex3f(-0.5,-0.5,-0.5);
        glTexCoord2f(1,0); glVertex3f(-0.5,-0.5, 0.5);
        glTexCoord2f(1,1); glVertex3f(-0.5, 0.5, 0.5);
        glTexCoord2f(0,1); glVertex3f(-0.5, 0.5,-0.5);
        // RIGHT
        glTexCoord2f(0,0); glVertex3f(0.5,-0.5,-0.5);
        glTexCoord2f(1,0); glVertex3f(0.5,-0.5, 0.5);
        glTexCoord2f(1,1); glVertex3f(0.5, 0.5, 0.5);
        glTexCoord2f(0,1); glVertex3f(0.5, 0.5,-0.5);
        // TOP
        glTexCoord2f(0,0); glVertex3f(-0.5,0.5,-0.5);
        glTexCoord2f(1,0); glVertex3f( 0.5,0.5,-0.5);
        glTexCoord2f(1,1); glVertex3f( 0.5,0.5, 0.5);
        glTexCoord2f(0,1); glVertex3f(-0.5,0.5, 0.5);
        // BOTTOM
        glTexCoord2f(0,0); glVertex3f(-0.5,-0.5,-0.5);
        glTexCoord2f(1,0); glVertex3f( 0.5,-0.5,-0.5);
        glTexCoord2f(1,1); glVertex3f( 0.5,-0.5, 0.5);
        glTexCoord2f(0,1); glVertex3f(-0.5,-0.5, 0.5);
        glEnd();

        glPopMatrix();

        glDisable(GL_TEXTURE_2D);

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

        glColor3f(b.r*0.65f, b.g*0.65f, b.b*0.65f);
        glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+0.4f,bz); glScalef(b.w*0.6f,0.8f,3.5f); glutSolidCube(1); glPopMatrix();

        if (i%4 == 0) {
            glColor3f(1.0f, 0.2f, 0.05f);
            glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+1.6f,bz); glScalef(b.w*0.85f,1.6f,0.2f); glutSolidCube(1); glPopMatrix();
            glColor3f(1,1,0.3f);
            glPushMatrix(); glTranslatef(bx,GROUND_Y+b.h+1.65f,bz+0.12f); glScalef(b.w*0.5f,0.6f,0.05f); glutSolidCube(1); glPopMatrix();
        }
    }
}
