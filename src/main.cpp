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
#include "../include/Boat.h"
#include "../include/Island.h"
#include "../include/Lighthouse.h"

static const float PI = 3.14159265f;

// Malla polar en disco: 80 anillos, 200 sectores, radio 50 (mar mas amplio). Al
// ser circular el borde queda a distancia constante en todas direcciones y la
// niebla lo funde de forma uniforme durante la orientacion y el desplazamiento.
Ocean ocean(80, 200, 50.0f);
Environment environment;
Boat boat(2.0f, 3.0f, 7.5f, 2.6f, 1.4f);

Island island(0.0f, -20.0f, 24.0f, 9.0f, 4.0f, 16, 64, 0.3f, 11);

Lighthouse lighthouse(island.getCenterX(), island.getPeakHeight(), island.getCenterZ());

int winW = 800, winH = 600;
float cameraX = 0.0f;
const float cameraY = 7.5f;
float cameraZ = 18.0f;
const float minYaw = -28.0f;
const float maxYaw = 28.0f;
const float minPitch = -16.0f;
const float maxPitch = 20.0f;
const float cameraMoveStep = 1.0f;
const float cameraSafeRadius = 24.0f;
float cameraYaw = 0.0f;
float cameraPitch = -6.0f;
float cameraFov = 50.0f;
bool dragging = false;
int lastX = 0, lastY = 0;
float simTime = 0.0f;

// Estado controlable por teclado
float animSpeed = 1.0f;   // multiplicador de velocidad de la animacion
bool lightingOn = true;   // iluminacion encendida/apagada
bool textureOn = true;    // textura del agua encendida/apagada

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

// La niebla se aplica solo a la geometria del mundo. Su color coincide con la
// banda general del horizonte de la boveda y nunca contamina el cielo ni el sol.
void initFog() {
    const GLfloat fogColor[] = { 0.62f, 0.30f, 0.39f, 1.0f };
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 42.0f);
    glFogf(GL_FOG_END, 70.0f);
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
    lighthouse.applyLight(simTime);   // foco giratorio del faro (GL_LIGHT1)
    environment.draw(cameraX, cameraY, cameraZ);

    // Toggle de iluminacion (tecla L): afecta a toda la geometria, no al cielo.
    if (lightingOn) glEnable(GL_LIGHTING);
    else            glDisable(GL_LIGHTING);

    glEnable(GL_FOG);
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
    glDisable(GL_FOG);

    // Haz de luz visible del faro (translucido, se dibuja al final)
    lighthouse.drawBeam(simTime);

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
    // Zoom optico: la posicion de la camara no cambia.
    if (key == '+' || key == '=') cameraFov = fmaxf(32.0f, cameraFov - 3.0f);
    if (key == '-' || key == '_') cameraFov = fminf(60.0f, cameraFov + 3.0f);

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

void moveCamera(float deltaX, float deltaZ) {
    const float nextX = cameraX + deltaX;
    const float nextZ = cameraZ + deltaZ;
    const float distance = sqrtf(nextX * nextX + nextZ * nextZ);

    if (distance <= cameraSafeRadius) {
        cameraX = nextX;
        cameraZ = nextZ;
    } else if (distance > 0.0001f) {
        const float scale = cameraSafeRadius / distance;
        cameraX = nextX * scale;
        cameraZ = nextZ * scale;
    }
}

// Las flechas trasladan la camara en el plano XZ segun el yaw actual. La altura
// permanece fija y la posicion se limita a una zona segura dentro del disco.
void special(int key, int x, int y) {
    const float yaw = cameraYaw * PI / 180.0f;
    const float forwardX = sinf(yaw);
    const float forwardZ = -cosf(yaw);
    const float rightX = cosf(yaw);
    const float rightZ = sinf(yaw);

    if (key == GLUT_KEY_UP)
        moveCamera(forwardX * cameraMoveStep, forwardZ * cameraMoveStep);
    if (key == GLUT_KEY_DOWN)
        moveCamera(-forwardX * cameraMoveStep, -forwardZ * cameraMoveStep);
    if (key == GLUT_KEY_LEFT)
        moveCamera(-rightX * cameraMoveStep, -rightZ * cameraMoveStep);
    if (key == GLUT_KEY_RIGHT)
        moveCamera(rightX * cameraMoveStep, rightZ * cameraMoveStep);
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
    initFog();

    // Espectro con rango de frecuencias amplio y dispersion direccional.
    // (El original data/spectrum.txt se conserva; cambia esta ruta para volver a el.)
    ocean.loadWaves("data/spectrum_realista.txt");
    ocean.loadTexture("assets/textures/ocean.tga");
    // Inicializa alturas, normales y maxHeight antes de dibujar el primer frame;
    // el barco consulta exactamente este mismo estado temporal desde el inicio.
    ocean.update(simTime);

    std::cout << "\n=== Controles ===\n"
              << "  Arrastrar mouse : rotar camara\n"
              << "  + / -           : acercar / alejar\n"
              << "  Flechas         : mover camara en el plano del mar\n"
              << "  W / S           : subir / bajar velocidad de la animacion\n"
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
