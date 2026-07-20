#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "../include/Ocean.h"
#include "../include/Environment.h"

static const float PI = 3.14159265f;

// Malla polar en disco: 80 anillos, 200 sectores, radio 50 (mar mas amplio). Al
// ser circular el borde queda a distancia constante en todas direcciones, la
// niebla lo funde uniforme y el mar parece infinito (sin esquinas al orbitar).
Ocean ocean(80, 200, 50.0f);
Environment environment;

int winW = 800, winH = 600;
float camRadius = 25.0f, camTheta = 35.0f, camPhi = 25.0f;
bool dragging = false;
int lastX = 0, lastY = 0;
float simTime = 0.0f;

// Estado controlable por teclado
float animSpeed = 1.0f;   // multiplicador de velocidad de la animacion
bool lightingOn = true;   // iluminacion encendida/apagada
bool textureOn = true;    // textura del agua encendida/apagada

void initWaterMaterial() {
    GLfloat ambient[]  = { 0.10f, 0.20f, 0.28f, 1.0f };
    GLfloat diffuse[]  = { 0.25f, 0.55f, 0.75f, 1.0f };
    // Especular bajo y muy concentrado: ahora que se suma DESPUES de la textura
    // (SEPARATE_SPECULAR_COLOR), valores altos sobreexponen. Menos intensidad y
    // mas shininess => un destello pequeno del sol, no una mancha blanca.
    GLfloat specular[] = { 0.12f, 0.13f, 0.14f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 110.0f);
}

// Niebla lineal: el agua se tine del color del horizonte a medida que se aleja,
// fundiendo el borde de la malla con el cielo (color = banda del horizonte).
void initFog() {
    GLfloat fogColor[] = { 0.88f, 0.75f, 0.57f, 1.0f };
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 40.0f);   // mas mar cercano nitido antes de la bruma
    glFogf(GL_FOG_END, 72.0f);     // el borde del disco (~75u) queda totalmente fundido
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

    // Toggle de iluminacion (tecla L): con la luz apagada la malla se ve plana
    if (lightingOn) glEnable(GL_LIGHTING);
    else            glDisable(GL_LIGHTING);

    // La niebla se activa solo para el oceano (el cielo 2D ya se dibujo antes)
    glEnable(GL_FOG);
    ocean.draw();
    glDisable(GL_FOG);

    glutSwapBuffers();
}

void timer(int) {
    // La velocidad de la animacion escala cuanto avanza el tiempo por frame
    simTime += 0.016f * animSpeed;
    ocean.update(simTime);
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    // Zoom de camara
    if (key == '+' || key == '=') camRadius = fmaxf(3.0f, camRadius * 0.9f);
    if (key == '-' || key == '_') camRadius = fminf(80.0f, camRadius * 1.1f);

    // Velocidad de la animacion
    if (key == 'w' || key == 'W') animSpeed = fminf(4.0f, animSpeed + 0.25f);
    if (key == 's' || key == 'S') animSpeed = fmaxf(0.0f, animSpeed - 0.25f);
    if (key == ' ') animSpeed = (animSpeed > 0.0f) ? 0.0f : 1.0f;  // pausa/reanuda

    // Activar / desactivar textura del agua
    if (key == 't' || key == 'T') {
        textureOn = !textureOn;
        ocean.setTexture(textureOn);
    }

    // Activar / desactivar iluminacion
    if (key == 'l' || key == 'L') lightingOn = !lightingOn;

    if (key == 27 || key == 'q' || key == 'Q') exit(0);
    glutPostRedisplay();
}

// Flechas arriba/abajo: tambien ajustan la velocidad de la animacion
void special(int key, int x, int y) {
    if (key == GLUT_KEY_UP)   animSpeed = fminf(4.0f, animSpeed + 0.25f);
    if (key == GLUT_KEY_DOWN) animSpeed = fmaxf(0.0f, animSpeed - 0.25f);
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
    initFog();

    // Espectro con rango de frecuencias amplio y dispersion direccional.
    // (El original data/spectrum.txt se conserva; cambia esta ruta para volver a el.)
    ocean.loadWaves("data/spectrum_realista.txt");
    ocean.loadTexture("assets/textures/ocean.tga");

    std::cout << "\n=== Controles ===\n"
              << "  Arrastrar mouse : rotar camara\n"
              << "  + / -           : acercar / alejar\n"
              << "  W / S (o flechas): subir / bajar velocidad de la animacion\n"
              << "  Barra espaciadora: pausar / reanudar\n"
              << "  T               : activar / desactivar textura\n"
              << "  L               : activar / desactivar iluminacion\n"
              << "  Q / Esc         : salir\n" << std::endl;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
