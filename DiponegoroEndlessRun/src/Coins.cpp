#include "../header/Coins.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include <GL/glut.h>
void drawCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].active) continue;
        Coin& c = coins[i];
        glPushMatrix();
        glTranslatef(c.x, c.y, c.z);
        glRotatef(c.rot, 0,1,0);

        glColor3f(1.0f, 0.82f, 0.08f);
        glPushMatrix(); glScalef(0.58f,0.58f,0.12f); glutSolidCube(1); glPopMatrix();

        glColor3f(0.95f,0.75f,0.05f);
        glPushMatrix(); glScalef(0.38f,0.38f,0.14f); glutSolidCube(1); glPopMatrix();

        glDisable(GL_LIGHTING);
        glColor4f(1.0f,1.0f,0.7f,0.7f);
        glPushMatrix(); glTranslatef(0.08f,0.08f,0.07f); glutSolidSphere(0.10f,6,4); glPopMatrix();
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }
}
