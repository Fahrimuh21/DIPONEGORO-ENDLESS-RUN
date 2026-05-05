#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

// ===== STATE =====
enum GameState { START, PLAYING, GAMEOVER };
GameState gameState = START;

// ===== PLAYER =====
int lane = 0;
float playerX = 0.0f;
float playerY = 0.0f;
float velocityY = 0.0f;
bool isJumping = false;
float legAngle = 0.0f;

// ===== WORLD =====
float worldOffset = 0.0f;

// ===== OBSTACLE =====
float obstacleZ = -50.0f;
int obstacleLane = 0;

// ===== GAME =====
int score = 0;

// ===== TEXT =====
void drawText(float x, float y, const char* text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1,1,1);
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ===== LANE =====
float laneToX(int l) {
    if (l == -1) return -3.0f;
    if (l == 1) return 3.0f;
    return 0.0f;
}

// ===== SKY =====
void drawSky() {
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glColor3f(0.1,0.1,0.3);
    glVertex3f(-100,50,-100);
    glVertex3f(100,50,-100);
    glVertex3f(100,50,100);
    glVertex3f(-100,50,100);
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

// ===== CLOUDS =====
void drawClouds() {
    glDisable(GL_DEPTH_TEST);
    glColor3f(1,1,1);

    for(int i=0;i<5;i++){
        float x=-20+i*10;
        float z=-30+i*15;

        glPushMatrix();
        glTranslatef(x,20,z);

        glScalef(3,1.5,1.5);
        glutSolidCube(1);

        glPopMatrix();
    }

    glEnable(GL_DEPTH_TEST);
}

// ===== PLAYER =====
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, 5.0f);

    glColor3f(0,0,1);
    glPushMatrix();
    glScalef(1.5,2,1);
    glutSolidCube(1);
    glPopMatrix();

    glColor3f(1,0.8,0.6);
    glPushMatrix();
    glTranslatef(0,1.8,0);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1.3,0.5,0);
    glRotatef(-sin(legAngle*0.1)*40,1,0,0);
    glScalef(0.5,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.3,0.5,0);
    glRotatef(sin(legAngle*0.1)*40,1,0,0);
    glScalef(0.5,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    glColor3f(0.2,0.2,0.2);

    glPushMatrix();
    glTranslatef(-0.5,-1,0);
    glRotatef(sin(legAngle*0.1)*50,1,0,0);
    glScalef(0.6,1.5,0.6);
    glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5,-1,0);
    glRotatef(-sin(legAngle*0.1)*50,1,0,0);
    glScalef(0.6,1.5,0.6);
    glutSolidCube(1);
    glPopMatrix();

    glPopMatrix();
}

// ===== OBSTACLE =====
void drawObstacle() {
    glPushMatrix();
    glTranslatef(laneToX(obstacleLane), -1, obstacleZ);
    glColor3f(1,0,0);
    glScalef(2,2,2);
    glutSolidCube(1);
    glPopMatrix();
}

// ===== GROUND =====
void drawGround() {
    for(int i=0;i<20;i++){
        float z = 10 - i*10 + fmod(worldOffset,10);

        glColor3f(0.25,0.23,0.2);
        glBegin(GL_QUADS);
        glVertex3f(-6,-2,z);
        glVertex3f(6,-2,z);
        glVertex3f(6,-2,z-10);
        glVertex3f(-6,-2,z-10);
        glEnd();
    }
}

// ===== DISPLAY =====
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    drawSky();
    drawClouds();

    gluLookAt(playerX,4,12, playerX,0,-5, 0,1,0);

    drawGround();
    drawPlayer();
    drawObstacle();

    // ===== UI =====
    if (gameState == START) {
        drawText(280,350,"ENDLESS RUNNER");
        drawText(250,300,"Press SPACE to Start");
    }
    else if (gameState == GAMEOVER) {
        drawText(300,350,"GAME OVER");
        drawText(250,300,"Press R to Restart");
    }

    glutSwapBuffers();
}

// ===== UPDATE =====
void update(int value) {

    if (gameState == PLAYING) {

        legAngle += 5;

        float targetX = laneToX(lane);
        playerX += (targetX - playerX)*0.2f;

        if (isJumping) {
            playerY += velocityY;
            velocityY -= 0.006;
            if (playerY<=0){
                playerY=0;
                isJumping=false;
            }
        }

        worldOffset += 0.6f;
        obstacleZ += 0.6f;

        if (obstacleZ > 10) {
            obstacleZ = -50;
            obstacleLane = (rand()%3)-1;
            score++;
        }

        if (fabs(playerX - laneToX(obstacleLane))<1.5 &&
            fabs(obstacleZ-5)<1.5 &&
            playerY<1) {
            gameState = GAMEOVER;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(13, update, 0);
}

// ===== INPUT =====
void keyboard(int key,int x,int y){
    if (gameState != PLAYING) return;

    if(key==GLUT_KEY_LEFT && lane>-1) lane--;
    if(key==GLUT_KEY_RIGHT && lane<1) lane++;
}

void keyboardNormal(unsigned char key,int x,int y){

    if (gameState == START && key==' ') {
        gameState = PLAYING;
    }
    else if (gameState == GAMEOVER && key=='r') {
        gameState = START;

        // reset
        playerX=0;
        playerY=0;
        lane=0;
        worldOffset=0;
        obstacleZ=-50;
        score=0;
    }
    else if (gameState == PLAYING) {
        if(key==' ' && !isJumping){
            isJumping=true;
            velocityY=0.2;
        }
    }
}

// ===== INIT =====
void init(){
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(60,800.0/600.0,1,200);
    glMatrixMode(GL_MODELVIEW);
}

// ===== MAIN =====
int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Runner with Start & Game Over");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutKeyboardFunc(keyboardNormal);
    glutTimerFunc(0,update,0);

    glutMainLoop();
    return 0;
}
