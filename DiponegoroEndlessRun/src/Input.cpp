#include "../header/Input.h"
#include "../header/Globals.h"
#include "../header/Game.h"
#include "../header/Init.h"
#include <stdio.h>
#include <stdlib.h>

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

void keyNormal(unsigned char key, int x, int y) {
    if (key == 27) exit(0);

    if (gameState == INTRO) {
        if (key == '\r' || key == '\n') { if (introTimer >= 4.0f) gameState = LOGIN; }
        return;
    }

    if (gameState == LOGIN) {
        if ((key == '\r' || key == '\n')) {
            if (usernameLen == 0) { loginError = true; sprintf(loginMsg, "Username cannot be empty!"); }
            else { loginError = false; gameState  = MENU; }
            return;
        }
        if (key == 8 || key == 127) { if (usernameLen > 0) { usernameLen--; username[usernameLen] = '\0'; } return; }
        if (key >= 32 && key < 127 && usernameLen < USERNAME_MAX-1) { username[usernameLen++] = key; username[usernameLen]   = '\0'; loginError = false; }
        return;
    }

    if (gameState == MENU) {
        if (showControls) { if (key == 8 || key == 127) showControls = false; return; }
        if (key == '\r' || key == '\n') {
            switch (menuSelection) {
                case 0: initGame(); gameState = PLAYING; break;
                case 1: cameraMode = (CameraMode)((cameraMode+1)%3); break;
                case 2: showControls = true; break;
                case 3: exit(0); break;
            }
        }
        return;
    }

    if (gameState == GAMEOVER) {
        if (key == 'r' || key == 'R') { initGame(); gameState = PLAYING; }
        if (key == 'm' || key == 'M') { gameState = MENU; menuSelection = 0; showControls = false; }
        return;
    }

    if (gameState == PLAYING) {
        if (key == ' ' && !isJumping && !isSliding) { isJumping  = true; playerVelY = 0.30f; }
        if ((key=='s'||key=='S') && !isJumping) { isSliding=true; slideTimer=1.0f; }
        if (key=='a'||key=='A') { if (playerLane>-1) playerLane--; }
        if (key=='d'||key=='D') { if (playerLane< 1) playerLane++; }
        if (key=='c'||key=='C') cameraMode=(CameraMode)((cameraMode+1)%3);
    }
}
