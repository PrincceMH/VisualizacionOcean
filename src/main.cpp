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

// Configura iluminacion y material del oceano (realista)
void initLuzYMaterial() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    // --- FUENTE DE LUZ (luz que brilla desde afuera) ---
    GLfloat luzAmbiental[]  = { 0.25f, 0.25f, 0.28f, 1.0f };  // grisáceo neutro
    GLfloat luzDifusa[]     = { 0.85f, 0.85f, 0.80f, 1.0f };  // ligeramente amarilla
    GLfloat luzEspecular[]  = { 1.0f,  1.0f,  1.0f, 1.0f };   // blanca pura
    GLfloat posicionLuz[]   = { 10.0f, 15.0f, 10.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiental);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);

    // --- MATERIAL DEL OCEANO (como refleja la luz el agua) ---
    // Componente Ambiente: color base en sombra (azul profundo)
    GLfloat waterAmbient[]  = { 0.05f, 0.10f, 0.15f, 1.0f };
    
    // Componente Difusa: color bajo iluminacion directa (azul agua)
    GLfloat waterDiffuse[]  = { 0.10f, 0.35f, 0.55f, 1.0f };
    
    // Componente Especular: brillo/reflejo (plateado con matiz azul)
    GLfloat waterSpecular[] = { 0.90f, 0.90f, 0.85f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   waterAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   waterDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  waterSpecular);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 90.0f);
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

    // Actualiza posicion de luz despues de gluLookAt (anclada al mundo)
    GLfloat posicionLuz[] = { 10.0f, 15.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);

    // NO usamos glColor3f aqui porque el material ya esta definido en initLuzYMaterial()
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