#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include "../include/Ocean.h"
#include "../include/Environment.h"
#include "../include/Boat.h"
#include "../include/Island.h"
#include "../include/Lighthouse.h"

static const float PI = 3.14159265f;

Ocean ocean(100, 100, 0.5f);
Environment environment;

Boat boat(2.0f, 3.0f, 7.5f, 2.6f, 1.4f);

Island island(0.0f, -20.0f, 24.0f, 9.0f, 4.0f, 16, 64, 0.3f, 11);

Lighthouse lighthouse(island.getCenterX(), island.getPeakHeight(), island.getCenterZ());

// Variables de camara orbital
int winW = 800, winH = 600;
const float cameraX = 0.0f;
const float cameraY = 7.5f;
const float cameraZ = 18.0f;
const float minYaw = -28.0f;
const float maxYaw = 28.0f;
const float minPitch = -16.0f;
const float maxPitch = 20.0f;
float cameraYaw = 0.0f;
float cameraPitch = -6.0f;
float cameraFov = 50.0f;
bool dragging = false;
int lastX = 0, lastY = 0;

float simTime = 0.0f;

void initWaterMaterial() {
    const GLfloat ambient[]  = { 0.16f, 0.28f, 0.40f, 1.0f };
    const GLfloat diffuse[]  = { 0.12f, 0.36f, 0.52f, 1.0f };
    const GLfloat specular[] = { 0.72f, 0.62f, 0.52f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 38.0f);
}

void initObjectMaterial() {
    const GLfloat specular[] = { 0.42f, 0.32f, 0.24f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 24.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(cameraFov, (float)winW / (float)winH, 0.1f, 150.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const float deg = PI / 180.0f;
    const float yaw = cameraYaw * deg;
    const float pitch = cameraPitch * deg;
    const float forwardX = sinf(yaw) * cosf(pitch);
    const float forwardY = sinf(pitch);
    const float forwardZ = -cosf(yaw) * cosf(pitch);

    gluLookAt(cameraX, cameraY, cameraZ,
              cameraX + forwardX,
              cameraY + forwardY,
              cameraZ + forwardZ,
              0.0, 1.0, 0.0);

    environment.applyLight();
    environment.draw(cameraX, cameraY, cameraZ);

    glDisable(GL_COLOR_MATERIAL);
    initWaterMaterial();
    ocean.draw();

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    initObjectMaterial();
    island.draw();
    lighthouse.draw();
    boat.draw(ocean, simTime);
    glDisable(GL_COLOR_MATERIAL);

    glutSwapBuffers();
}

void timer(int) {
    simTime += 0.016f;
    miOceano.update(simTime);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '+' || key == '=') cameraFov = fmaxf(32.0f, cameraFov - 3.0f);
    if (key == '-' || key == '_') cameraFov = fminf(60.0f, cameraFov + 3.0f);
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

    cameraYaw += (x - lastX) * 0.25f;
    cameraPitch -= (y - lastY) * 0.25f;

    cameraYaw = fmaxf(minYaw, fminf(maxYaw, cameraYaw));
    cameraPitch = fmaxf(minPitch, fminf(maxPitch, cameraPitch));

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
    glutCreateWindow("Proyecto Oceano - Atardecer");

    glClearColor(0.10f, 0.12f, 0.20f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    environment.initLight();

    miOceano.loadWaves("data/spectrum.txt");
    miOceano.loadTexture("assets/textures/ocean.tga");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    
    return 0;
}