#include "../header/UI.h"
#include "../header/Camera.h"
#include "../header/Globals.h"
#include "../header/Utility.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

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

void drawText(float x, float y, const char* text, void* font) {
    if (!font) font = GLUT_BITMAP_HELVETICA_18;
    glRasterPos2f(x, y);
    for (int i = 0; text[i]; i++) glutBitmapCharacter(font, text[i]);
}

int textWidth(const char* text, void* font) {
    if (!font) font = GLUT_BITMAP_HELVETICA_18;
    return glutBitmapLength(font, (const unsigned char*)text);
}

void drawTextCentered(float y, const char* text, void* font) {
    float x = (WINDOW_W - textWidth(text, font)) * 0.5f;
    drawText(x, y, text, font);
}

void drawPanel(float x1, float y1, float x2, float y2, float r, float g, float b, float a, bool border) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS); glVertex2f(x1,y1); glVertex2f(x2,y1); glVertex2f(x2,y2); glVertex2f(x1,y2); glEnd();
    if (border) { glColor4f(1,1,1,0.3f); glLineWidth(1.5f); glBegin(GL_LINE_LOOP); glVertex2f(x1,y1); glVertex2f(x2,y1); glVertex2f(x2,y2); glVertex2f(x1,y2); glEnd(); }
}

void drawMenuButton(float cx, float cy, float w, float h, const char* label, bool selected, void* font) {
    float x1 = cx - w*0.5f, x2 = cx + w*0.5f; float y1 = cy - h*0.5f, y2 = cy + h*0.5f;
    if (selected) glColor4f(0.2f, 0.6f, 1.0f, 0.85f); else glColor4f(0.1f, 0.1f, 0.2f, 0.7f);
    glBegin(GL_QUADS); glVertex2f(x1,y1); glVertex2f(x2,y1); glVertex2f(x2,y2); glVertex2f(x1,y2); glEnd();
    glColor4f(selected?1.0f:0.5f, selected?1.0f:0.5f, 1.0f, 0.6f); glLineWidth(selected?2.0f:1.0f); glBegin(GL_LINE_LOOP); glVertex2f(x1,y1); glVertex2f(x2,y1); glVertex2f(x2,y2); glVertex2f(x1,y2); glEnd();
    if (selected) glColor3f(1.0f, 1.0f, 1.0f); else glColor3f(0.75f, 0.75f, 0.85f);
    float tx = cx - textWidth(label, font)*0.5f; drawText(tx, cy - 6, label, font);
}

void drawHUD() {
    begin2D();
    drawPanel(0, 565, WINDOW_W, WINDOW_H, 0.0f,0.0f,0.0f,0.55f, false);
    char buf[128];
    glColor3f(0.65f,0.85f,1.0f); sprintf(buf, "Player: %s", username); drawText(16, 575, buf, GLUT_BITMAP_HELVETICA_12);
    glColor3f(1.0f,0.85f,0.1f); sprintf(buf, "Score: %d", score); drawText(180, 575, buf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f,1.0f,0.35f); sprintf(buf, "Coins: %d", coinCount); drawText(360, 575, buf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.45f,1.0f,0.55f); sprintf(buf, "Speed: x%.1f", gameSpeed/0.55f); drawText(540, 575, buf, GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.8f,0.6f,1.0f); sprintf(buf, "Cam: %s [C]", cameraName()); drawText(680, 575, buf, GLUT_BITMAP_HELVETICA_12);
    if (isSliding) { glColor4f(0.2f,0.9f,1.0f,0.9f); drawTextCentered(48, "SLIDING!", GLUT_BITMAP_HELVETICA_18); }
    if (isJumping) { glColor4f(0.5f,1.0f,0.5f,0.9f); drawTextCentered(48, "JUMPING!", GLUT_BITMAP_HELVETICA_12); }
    end2D();
}

void drawIntro() {
    begin2D();
    glColor4f(0.04f,0.04f,0.12f,1.0f); glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_W,0); glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H); glEnd();
    float pulse = 0.85f + 0.15f*sinf(introTimer*3.0f);
    glColor4f(0.0f,0.0f,0.0f,0.7f); drawTextCentered(342, "DIPONEGORO ENDLESS RUN", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(1.0f*pulse, 0.82f*pulse, 0.05f*pulse); drawTextCentered(345, "DIPONEGORO ENDLESS RUN", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.7f,0.7f,0.75f); drawTextCentered(295, "An OpenGL Endless Runner", GLUT_BITMAP_HELVETICA_12);
    float prog = clampf(introTimer/4.0f, 0.0f, 1.0f);
    drawPanel(200, 228, 700, 248, 0.1f,0.1f,0.15f,0.9f, true);
    glColor4f(0.2f,0.65f,1.0f,1.0f); glBegin(GL_QUADS); glVertex2f(202,230); glVertex2f(202+496*prog,230); glVertex2f(202+496*prog,246); glVertex2f(202,246); glEnd();
    if (introTimer >= 4.0f) { glColor4f(0.85f,0.85f,0.9f, 0.5f+0.5f*sinf(introTimer*4.0f)); drawTextCentered(195, "Press ENTER to continue", GLUT_BITMAP_HELVETICA_12); }
    glColor3f(0.35f,0.35f,0.4f); drawText(8, 10, "v2.0 Enhanced Edition", GLUT_BITMAP_HELVETICA_12);
    end2D();
}

void drawLogin() {
    begin2D(); glColor4f(0.04f,0.04f,0.12f,1.0f); glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_W,0); glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H); glEnd();
    drawPanel(250,200,650,430, 0.06f,0.06f,0.18f,0.92f);
    glColor3f(1.0f,0.82f,0.08f); drawTextCentered(400, "DIPONEGORO ENDLESS RUN", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.7f,0.75f,0.85f); drawTextCentered(368, "Enter your username to continue", GLUT_BITMAP_HELVETICA_12);
    bool blink = (int)(introTimer*2)%2==0;
    drawPanel(285,310,615,345, 0.1f,0.1f,0.22f,0.95f);
    glColor3f(0.4f,0.75f,1.0f); glLineWidth(2.0f); glBegin(GL_LINE_LOOP); glVertex2f(285,310); glVertex2f(615,310); glVertex2f(615,345); glVertex2f(285,345); glEnd();
    char displayName[USERNAME_MAX+2]; sprintf(displayName, "%s%s", username, blink?"|":""); glColor3f(1.0f,1.0f,1.0f); drawText(295, 322, displayName, GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.6f,0.7f,0.85f); drawText(295, 358, "Username:", GLUT_BITMAP_HELVETICA_12);
    drawMenuButton(450, 278, 120, 32, "[ ENTER ]", true, GLUT_BITMAP_HELVETICA_12);
    if (loginError) { glColor3f(1.0f, 0.3f, 0.3f); drawTextCentered(248, loginMsg, GLUT_BITMAP_HELVETICA_12); }
    glColor3f(0.4f,0.45f,0.55f); drawTextCentered(218, "Press ENTER to confirm", GLUT_BITMAP_HELVETICA_12);
    end2D();
}

void drawMenu() {
    begin2D(); glColor4f(0.04f,0.04f,0.12f,0.92f); glBegin(GL_QUADS); glVertex2f(0,0); glVertex2f(WINDOW_W,0); glVertex2f(WINDOW_W,WINDOW_H); glVertex2f(0,WINDOW_H); glEnd();
    drawPanel(220,410,680,490, 0.05f,0.05f,0.18f,0.92f); glColor3f(1.0f,0.82f,0.08f); drawTextCentered(455, "DIPONEGORO ENDLESS RUN", GLUT_BITMAP_TIMES_ROMAN_24);
    char greet[64]; sprintf(greet, "Welcome, %s!", username); glColor3f(0.65f,0.8f,1.0f); drawTextCentered(428, greet, GLUT_BITMAP_HELVETICA_12);
    if (!showControls) {
        drawPanel(260,120,640,400, 0.06f,0.06f,0.18f,0.88f);
        float btnY[4] = {355, 295, 235, 160};
        for (int i=0; i<MENU_COUNT; i++) drawMenuButton(450, btnY[i], 260, 44, menuItems[i], i==menuSelection, GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.4f,0.45f,0.55f); drawTextCentered(128, "UP/DOWN to navigate  |  ENTER to select", GLUT_BITMAP_HELVETICA_12);
        if (menuSelection==1) { glColor3f(0.8f,0.6f,1.0f); char camBuf[48]; sprintf(camBuf, "Current: %s   (ENTER to cycle)", cameraName()); drawTextCentered(108, camBuf, GLUT_BITMAP_HELVETICA_12); }
    } else {
        drawPanel(200,80,700,400, 0.06f,0.06f,0.18f,0.92f);
        glColor3f(1.0f,0.85f,0.1f); drawTextCentered(378, "CONTROLS", GLUT_BITMAP_HELVETICA_18);
        struct CtrlEntry { const char* key; const char* action; } ctrls[] = {{"LEFT / A","Move lane left"},{"RIGHT / D","Move lane right"},{"SPACE","Jump"},{"DOWN / S","Slide"},{"C","Toggle camera"},{"R","Restart (Game Over)"},{"ESC","Quit"}};
        int nc = 7;
        for (int i=0; i<nc; i++) { float cy2 = 345.0f - i*38.0f; glColor3f(0.4f,0.75f,1.0f); drawText(230, cy2, ctrls[i].key, GLUT_BITMAP_HELVETICA_12); glColor3f(0.85f,0.85f,0.9f); drawText(430, cy2, ctrls[i].action, GLUT_BITMAP_HELVETICA_12); }
        glColor3f(0.5f,1.0f,0.5f); drawTextCentered(98, "Press BACKSPACE to go back", GLUT_BITMAP_HELVETICA_12);
    }
    end2D();
}

void drawGameOver() {
    begin2D(); drawPanel(160,120,740,490, 0.08f,0.0f,0.0f,0.92f);
    glColor3f(1.0f,0.15f,0.15f); drawTextCentered(450, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);
    char buf[128]; glColor3f(0.65f,0.85f,1.0f); sprintf(buf, "Player: %s", username); drawTextCentered(412, buf, GLUT_BITMAP_HELVETICA_18);
    glColor4f(1,1,1,0.2f); glBegin(GL_LINES); glVertex2f(200,402); glVertex2f(700,402); glEnd();
    glColor3f(1.0f,0.85f,0.1f); sprintf(buf, "Final Score:  %d", score); drawTextCentered(370, buf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(1.0f,1.0f,0.35f); sprintf(buf, "Coins Collected:  %d", coinCount); drawTextCentered(335, buf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.55f,1.0f,0.55f); sprintf(buf, "Top Speed:  x%.2f", topSpeed/0.55f); drawTextCentered(298, buf, GLUT_BITMAP_HELVETICA_12);
    glColor4f(1,1,1,0.2f); glBegin(GL_LINES); glVertex2f(200,282); glVertex2f(700,282); glEnd();
    float blink = 0.65f + 0.35f*sinf(introTimer*3.0f);
    glColor3f(0.3f*blink, 1.0f*blink, 0.35f*blink); drawTextCentered(240, "Press  R  to Restart", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.6f,0.6f,0.65f); drawTextCentered(188, "Press  M  to go to Main Menu", GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.45f,0.45f,0.5f); drawTextCentered(158, "Press  ESC  to Quit", GLUT_BITMAP_HELVETICA_12);
    end2D();
}
