#include "../header/Camera.h"
#include "../header/Globals.h"
#include <GL/glut.h>

void setupCamera() {
    float px = playerX;
    float py = GROUND_Y + playerY + 1.2f;
    float pz = 5.5f;

    switch (cameraMode) {
        case CAM_BACK:
            gluLookAt(px*0.45f, 6.5f, 15.5f, px*0.25f, 0.5f, -5.0f, 0,1,0);
            break;
        case CAM_SIDE:
            gluLookAt(18.0f, 4.5f, pz, px, py-0.5f, pz, 0,1,0);
            break;
        case CAM_FIRST:
            gluLookAt(px, py + 0.8f*(isSliding?0.3f:1.0f), pz, px*0.85f, py - 0.5f, pz - 18.0f, 0,1,0);
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
