#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

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
bool gameOver = false;
int score = 0;

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

    // atas
    glColor3f(0.1f, 0.1f, 0.3f);
    glVertex3f(-100, 50, -100);
    glVertex3f(100, 50, -100);
    glVertex3f(100, 50, 100);
    glVertex3f(-100, 50, 100);

    // belakang
    glColor3f(0.05f, 0.05f, 0.2f);
    glVertex3f(-100, -2, -100);
    glVertex3f(100, -2, -100);
    glVertex3f(100, 50, -100);
    glVertex3f(-100, 50, -100);

    glEnd();

    glEnable(GL_DEPTH_TEST);
}

// ===== CLOUDS =====
void drawClouds() {
    glDisable(GL_DEPTH_TEST);
    glColor3f(1,1,1);

    for (int i = 0; i < 5; i++) {
        float x = -20 + i * 10;
        float z = -30 + i * 15;

        glPushMatrix();
        glTranslatef(x, 20, z);

        glPushMatrix();
        glScalef(3,1.5,1.5);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(1.5,0.5,0);
        glScalef(2,1,1);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-1.5,0.5,0);
        glScalef(2,1,1);
        glutSolidCube(1);
        glPopMatrix();

        glPopMatrix();
    }

    glEnable(GL_DEPTH_TEST);
}

// ===== PLAYER =====
void drawPlayer() {
    glPushMatrix();
    glTranslatef(playerX, playerY, 5.0f);

    // badan
    glColor3f(0,0,1);
    glPushMatrix();
    glScalef(1.5,2,1);
    glutSolidCube(1);
    glPopMatrix();

    // kepala
    glColor3f(1,0.8,0.6);
    glPushMatrix();
    glTranslatef(0,1.8,0);
    glutSolidCube(1);
    glPopMatrix();

    // tangan kiri
    glPushMatrix();
    glTranslatef(-1.3,0.5,0);
    glTranslatef(0,0.75,0);
    glRotatef(-sin(legAngle*0.1)*40,1,0,0);
    glTranslatef(0,-0.75,0);
    glScalef(0.5,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    // tangan kanan
    glPushMatrix();
    glTranslatef(1.3,0.5,0);
    glTranslatef(0,0.75,0);
    glRotatef(sin(legAngle*0.1)*40,1,0,0);
    glTranslatef(0,-0.75,0);
    glScalef(0.5,1.5,0.5);
    glutSolidCube(1);
    glPopMatrix();

    // kaki kiri
    glColor3f(0.2,0.2,0.2);
    glPushMatrix();
    glTranslatef(-0.5,-1,0);
    glTranslatef(0,0.75,0);
    glRotatef(sin(legAngle*0.1)*50,1,0,0);
    glTranslatef(0,-0.75,0);
    glScalef(0.6,1.5,0.6);
    glutSolidCube(1);
    glPopMatrix();

    // kaki kanan
    glPushMatrix();
    glTranslatef(0.5,-1,0);
    glTranslatef(0,0.75,0);
    glRotatef(-sin(legAngle*0.1)*50,1,0,0);
    glTranslatef(0,-0.75,0);
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
    for (int i=0;i<20;i++) {
        float z = 10 - i*10 + fmod(worldOffset,10);

        // jalan utama
        glColor3f(0.25,0.23,0.2);
        glBegin(GL_QUADS);
        glVertex3f(-6,-2,z);
        glVertex3f(6,-2,z);
        glVertex3f(6,-2,z-10);
        glVertex3f(-6,-2,z-10);
        glEnd();

        // garis lane
        glColor3f(1,1,1);
        glBegin(GL_LINES);
        glVertex3f(-2,-1.99,z);
        glVertex3f(-2,-1.99,z-10);
        glVertex3f(2,-1.99,z);
        glVertex3f(2,-1.99,z-10);
        glEnd();

        // bahu kiri
        glColor3f(0.4,0.3,0.2);
        glBegin(GL_QUADS);
        glVertex3f(-8,-2,z);
        glVertex3f(-6,-2,z);
        glVertex3f(-6,-2,z-10);
        glVertex3f(-8,-2,z-10);
        glEnd();

        // bahu kanan
        glBegin(GL_QUADS);
        glVertex3f(6,-2,z);
        glVertex3f(8,-2,z);
        glVertex3f(8,-2,z-10);
        glVertex3f(6,-2,z-10);
        glEnd();
    }
}

// ===== RAIL =====
void drawRails() {
    glColor3f(0.7,0.7,0.7);
    for (int i=0;i<20;i++) {
        float z = 10 - i*10 + fmod(worldOffset,10);

        glBegin(GL_LINES);
        glVertex3f(-1,-1.9,z);
        glVertex3f(-1,-1.9,z-10);
        glVertex3f(1,-1.9,z);
        glVertex3f(1,-1.9,z-10);
        glEnd();
    }
}

// ===== GEDUNG =====
void drawBuildings() {
    for (int i=0;i<20;i++) {
        float z = 10 - i*10 + fmod(worldOffset,10);
        float h = 4 + (i%4);

        glPushMatrix();
        glTranslatef(-10,-2+h/2,z-5);
        glColor3f(0.2+(i%3)*0.2,0.4,0.6);
        glScalef(3,h,3);
        glutSolidCube(1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(10,-2+h/2,z-5);
        glColor3f(0.6,0.3+(i%2)*0.3,0.3);
        glScalef(3,h+1,3);
        glutSolidCube(1);
        glPopMatrix();
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
    drawRails();
    drawBuildings();
    drawPlayer();
    drawObstacle();

    glutSwapBuffers();
}

// ===== UPDATE =====
void update(int value) {
    if (!gameOver) {

        legAngle += 5;

        float targetX = laneToX(lane);
        playerX += (targetX - playerX)*0.2f;

        if (isJumping) {
            playerY += velocityY;
            velocityY -= 0.006;
            if (playerY<=0) {
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
            gameOver = true;
            printf("Game Over! Score: %d\n",score);
        }
    }

    glutPostRedisplay();
    glutTimerFunc(13, update, 0);
}

// ===== INPUT =====
void keyboard(int key,int x,int y) {
    if (gameOver) return;
    if (key==GLUT_KEY_LEFT && lane>-1) lane--;
    if (key==GLUT_KEY_RIGHT && lane<1) lane++;
}

void keyboardNormal(unsigned char key,int x,int y) {
    if (key==' ' && !isJumping) {
        isJumping=true;
        velocityY=0.2;
    }
}

// ===== INIT =====
void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.05f,0.05f,0.1f,1.0f);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(60,800.0/600.0,1,200);
    glMatrixMode(GL_MODELVIEW);
}

// ===== MAIN =====
int main(int argc,char** argv) {
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Subway Runner + Sky + Clouds");

    init();

    glutDisplayFunc(display);
    glutSpecialFunc(keyboard);
    glutKeyboardFunc(keyboardNormal);
    glutTimerFunc(0,update,0);

    glutMainLoop();
    return 0;
}
