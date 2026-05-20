#include "header/Globals.h"
#include "header/Init.h"
#include "header/Input.h"
#include "header/Render.h"
#include "header/Game.h"
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    srand((unsigned int)time(0));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutCreateWindow("Diponegoro Endless Run");

    initGL();
    initObstacles();
    initCoins();
    initBuildings();
    initLamps();
    initClouds();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyNormal);
    glutSpecialFunc(keySpecial);
    glutTimerFunc(16, update, 0);

    glutMainLoop();
    return 0;
}
