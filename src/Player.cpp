#include "../header/Player.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include <GL/glut.h>
#include <math.h>

void drawPlayer() {
    float slideScale = isSliding ? 0.5f : 1.0f;
    float bodyH = 1.6f * squashY * slideScale;
    float headY = isSliding ? 0.3f : 1.55f;

    glPushMatrix();
    glTranslatef(playerX, GROUND_Y + playerY + 1.25f, 5.5f);
    glRotatef(0.0f, 0, 1, 0);

    glColor3f(0.08f, 0.52f, 0.95f);
    glPushMatrix(); glScalef(1.25f, bodyH, 0.85f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.95f, 0.2f, 0.1f);
    glPushMatrix(); glTranslatef(0, -0.6f*squashY*slideScale, 0); glScalef(1.27f, 0.15f, 0.86f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.98f, 0.80f, 0.62f);
    glPushMatrix(); glTranslatef(0, headY*squashY, 0); glScalef(0.88f,0.88f,0.88f); glutSolidCube(1); glPopMatrix();

    glColor3f(0.1f,0.1f,0.15f);
    for (int e=-1; e<=1; e+=2) { glPushMatrix(); glTranslatef(e*0.2f, headY*squashY+0.05f, -0.45f); drawSphere(0.08f,5,4); glPopMatrix(); }

    glColor3f(0.9f,0.08f,0.08f);
    glPushMatrix(); glTranslatef(0, headY*squashY+0.52f, -0.12f); glScalef(0.92f,0.22f,0.8f); glutSolidCube(1); glPopMatrix();
    glPushMatrix(); glTranslatef(0, headY*squashY+0.4f,  -0.5f); glScalef(0.55f,0.12f,0.3f); glutSolidCube(1); glPopMatrix();

    if (!isSliding) {
        glColor3f(0.08f, 0.52f, 0.95f);
        for (int side=-1; side<=1; side+=2) {
            glPushMatrix(); glTranslatef(side*0.95f, 0.3f, 0); glRotatef(sinf(armAngle) * side * -50.0f, 1,0,0); glTranslatef(0,-0.7f,0); glScalef(0.42f,1.35f,0.42f); glutSolidCube(1); glPopMatrix();
        }

        glColor3f(0.15f,0.15f,0.32f);
        for (int leg=-1; leg<=1; leg+=2) { glPushMatrix(); glTranslatef(leg*0.38f, -0.82f, 0); glRotatef(sinf(legAngle)*leg*-55.0f, 1,0,0); glTranslatef(0,-0.72f,0); glScalef(0.52f,1.45f,0.52f); glutSolidCube(1); glPopMatrix(); }

        glColor3f(0.9f,0.82f,0.08f);
        for (int s=-1; s<=1; s+=2) { glPushMatrix(); glTranslatef(s*0.38f, -2.02f, sinf(legAngle)*s*0.25f); glScalef(0.58f,0.32f,0.88f); glutSolidCube(1); glPopMatrix(); }
    } else {
        glColor3f(0.08f, 0.52f, 0.95f);
        for (int side=-1; side<=1; side+=2) { glPushMatrix(); glTranslatef(side*1.1f, 0.1f, 0); glScalef(0.42f,0.42f,1.3f); glutSolidCube(1); glPopMatrix(); }
    }

    glPopMatrix();
}
