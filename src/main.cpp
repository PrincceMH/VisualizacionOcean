#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>
#include "../include/Ocean.h"
#include "../include/Environment.h"

static const float PI = 3.14159265f;

Ocean ocean(100, 100, 0.5f);
Environment environment;

int winW = 800, winH = 600;
float camRadius = 25.0f, camTheta = 35.0f, camPhi = 25.0f;
bool dragging = false;
int lastX = 0, lastY = 0;
float simTime = 0.0f;

void initWaterMaterial() {
    GLfloat ambient[]  = { 0.05f, 0.10f, 0.15f, 1.0f };
    GLfloat diffuse[]  = { 0.10f, 0.35f, 0.55f, 1.0f };
    GLfloat specular[] = { 0.45f, 0.52f, 0.58f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 48.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    environment.draw(winW, winH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)winW / (float)winH, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float deg = PI / 180.0f;
    float ph = camPhi * deg;
    float th = camTheta * deg;
    float ex = camRadius * cosf(ph) * sinf(th);
    float ey = camRadius * sinf(ph) + 1.0f;
    float ez = camRadius * cosf(ph) * cosf(th);

    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    environment.applyLight();
    ocean.draw();

    glutSwapBuffers();
}

void timer(int) {
    simTime += 0.016f;
    ocean.update(simTime);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') camRadius = fmaxf(3.0f, camRadius * 0.9f);
    if (key == '-' || key == '_') camRadius = fminf(80.0f, camRadius * 1.1f);
    if (key == 27 || key == 'q' || key == 'Q') exit(0);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastX = x;
        lastY = y;
    }
}

void motion(int x, int y) {
    if (!dragging) return;

    camTheta += (x - lastX) * 0.4f;
    camPhi += (y - lastY) * 0.4f;
    if (camPhi > 89.0f) camPhi = 89.0f;
    if (camPhi < -10.0f) camPhi = -10.0f;

    lastX = x;
    lastY = y;
    glutPostRedisplay();
}

void reshape(int w, int h) {
    winW = w;
    winH = (h > 0) ? h : 1;
    glViewport(0, 0, winW, winH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Proyecto Oceano - Camara Orbital");

    glEnable(GL_DEPTH_TEST);
    environment.initLight();
    initWaterMaterial();

    ocean.loadWaves("data/spectrum.txt");
    ocean.loadTexture("assets/textures/ocean.tga");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
