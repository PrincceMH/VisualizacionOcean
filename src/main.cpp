#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include "../include/Ocean.h"

static const float PI = 3.14159265f;

Ocean miOceano(50, 50, 0.5f);

// Variables de camara orbital
int winW = 800, winH = 600;
float camRadius = 25.0f, camTheta = 35.0f, camPhi = 25.0f;
bool dragging = false;
int lastX = 0, lastY = 0;

float simTime = 0.0f;

// Configura una fuente de luz (ambiental + difusa + especular) y el
// material del oceano (con especular fuerte para simular brillos en las olas)
void initLuzYMaterial() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    GLfloat luzAmbiental[]  = { 0.25f, 0.25f, 0.3f, 1.0f };
    GLfloat luzDifusa[]     = { 0.8f,  0.8f,  0.8f, 1.0f };
    GLfloat luzEspecular[]  = { 1.0f,  1.0f,  1.0f, 1.0f };
    GLfloat posicionLuz[]   = { 10.0f, 15.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiental);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat matEspecular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, matEspecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 96.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)winW / (float)winH, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float DEG = PI / 180.0f;
    float ph = camPhi * DEG;
    float th = camTheta * DEG;
    float ex = camRadius * cosf(ph) * sinf(th);
    float ey = camRadius * sinf(ph) + 1.0f;
    float ez = camRadius * cosf(ph) * cosf(th);

    gluLookAt(ex, ey, ez,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);

    GLfloat posicionLuz[] = { 10.0f, 15.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);

    glColor3f(0.0f, 0.8f, 1.0f);
    miOceano.draw();

    glutSwapBuffers();
}

void timer(int) {
    simTime += 0.016f;
    miOceano.update(simTime);
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

    initLuzYMaterial();

    miOceano.loadWaves("data/spectrum.txt");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    
    return 0;
}