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
#include "../include/Rain.h"
#include "../include/stb_image.h"

static const float PI = 3.14159265f;

// CORRECCIÓN 1: Se agregó el punto y coma (;) al final de esta línea
GLuint texArena, texPasto, texMuro, texFaro, texCubierta; 

// Malla polar en disco: 80 anillos, 200 sectores, radio 50 (mar mas amplio). Al
// ser circular el borde queda a distancia constante en todas direcciones y la
// niebla lo funde de forma uniforme durante la orientacion y el desplazamiento.
Ocean ocean(120, 300, 400.0f);
Environment environment;
Boat boat(2.0f, 3.0f, 7.5f, 2.6f, 1.4f);
// (CentroX, CentroZ, RadioX, RadioZ, AlturaMax, Anillos, Sectores, Irregularidad, Semilla)
Island island(0.0f, -45.0f, 85.0f, 40.0f, 20.0f, 32, 128, 0.35f, 11);

// Le agregamos: 12.0f (altura), 1.8f (radio de la base), y 1.2f (radio superior)
Lighthouse lighthouse(island.getCenterX(), island.getPeakHeight(), island.getCenterZ(), 12.0f, 1.8f, 1.2f);

int winW = 800, winH = 600;
float cameraX = 0.0f;
const float cameraY = 7.5f;
float cameraZ = 45.0f;
// Expandimos el Yaw a valores inmensos para girar infinitamente
const float minYaw = -36000.0f; 
const float maxYaw = 36000.0f;
// Expandimos el Pitch pero lo limitamos a 89 grados para no quedar de cabeza
const float minPitch = -89.0f;  
const float maxPitch = 89.0f;
const float cameraMoveStep = 1.0f;
const float cameraSafeRadius = 100.0f;
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

// --- Modo tormenta ---
Rain rain(1500);           // sistema de lluvia (billboards)
bool  stormActive = false; // tecla P: activa/desactiva la tormenta
float stormT = 0.0f;       // factor 0..1 que interpola todo (transicion gradual ~2.5s)
float flash = 0.0f;        // intensidad del relampago actual (decae rapido)

float frand01() { return rand() / (float)RAND_MAX; }

GLuint loadTextureGeneral(const char* filename) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Configuración para que la textura se repita y se vea nítida
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
        std::cout << "Textura cargada exitosamente: " << filename << std::endl;
    } else {
        std::cerr << "Error al cargar la textura: " << filename << std::endl;
    }
    return texID;
}

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
    glFogf(GL_FOG_START, 200.0f);
    glFogf(GL_FOG_END, 390.0f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(cameraFov, (float)winW / (float)winH, 0.1f, 1000.0f);

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

    // El faro casi apagado de dia, encendido con fuerza en tormenta.
    const float faroIntensity = 0.15f + 0.85f * stormT;

    environment.applyLight();
    lighthouse.applyLight(faroIntensity);   // luz puntual del faro (GL_LIGHT1)
    environment.draw(cameraX, cameraY, cameraZ);

    // Toggle de iluminacion (tecla L)
    if (lightingOn) glEnable(GL_LIGHTING);
    else            glDisable(GL_LIGHTING);

    glEnable(GL_FOG);
    
    // 1. Dibujamos el Océano
    glDisable(GL_COLOR_MATERIAL);
    initWaterMaterial();
    ocean.draw();

    // 2. Dibujamos los objetos 3D UNA SOLA VEZ
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    initObjectMaterial();
    
    island.draw(texArena, texPasto);
    lighthouse.draw(texFaro);
    boat.draw(ocean, simTime, texCubierta);
    
    // Apagamos los estados
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_FOG);

    // Haz de luz visible del faro (giratorio, inclinado hacia la camara)
    lighthouse.drawBeam(simTime, cameraX, cameraY, cameraZ, faroIntensity);

    // Lluvia de tormenta (billboards), su alfa sube con stormT
    rain.draw(stormT);

    // Relampago: destello blanco a pantalla completa (aditivo) que decae rapido
    if (flash > 0.01f) {
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
        glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING); glDisable(GL_TEXTURE_2D); glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE);   // aditivo
        glColor4f(0.85f, 0.88f, 1.0f, flash * 0.55f);
        glBegin(GL_QUADS);
            glVertex2f(0, 0); glVertex2f(1, 0); glVertex2f(1, 1); glVertex2f(0, 1);
        glEnd();
        glPopAttrib();
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    }

    glutSwapBuffers();
}

void timer(int) {
    const float dt = 0.016f;

    // Transicion gradual del clima: stormT avanza hacia 1 (tormenta) o 0 (despejado)
    // a lo largo de ~2.5 s, de modo que todos los efectos entran/salen suavemente.
    const float target = stormActive ? 1.0f : 0.0f;
    const float rate = dt / 2.5f;
    if (stormT < target) stormT = fminf(target, stormT + rate);
    else                 stormT = fmaxf(target, stormT - rate);

    // En tormenta el tiempo avanza mas rapido (olas mas veloces) y la amplitud sube.
    simTime += dt * animSpeed * (1.0f + 0.8f * stormT);
    ocean.setWaveScale(1.0f + 3.0f * stormT);   // mar embravecido
    ocean.update(simTime);

    environment.setStorm(stormT);               // cielo/luna/iluminacion
    rain.update(dt, cameraX, cameraY, cameraZ); // lluvia (sigue a la camara)

    // Relampagos: solo con tormenta marcada. Baja probabilidad de disparo por frame;
    // el destello decae rapido (~0.15 s). A veces encadena un segundo fogonazo.
    if (stormT > 0.45f && flash < 0.05f && frand01() < 0.006f)
        flash = (frand01() < 0.4f) ? 0.7f : 1.0f;
    flash -= dt * 6.0f;
    if (flash < 0.0f) flash = 0.0f;

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

    // Activar / desactivar MODO TORMENTA (transicion gradual)
    if (key == 'p' || key == 'P') stormActive = !stormActive;

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

    // Cargar tus imágenes JPG
    texArena = loadTextureGeneral("assets/textures/arena.jpg");
    texPasto = loadTextureGeneral("assets/textures/pasto.jpg");
    texMuro = loadTextureGeneral("assets/textures/muro.jpg");
    texFaro = loadTextureGeneral("assets/textures/faro.jpg");
    texCubierta = loadTextureGeneral("assets/textures/cubierta.jpg");

    rain.initTexture();   // textura procedural de las gotas de lluvia

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
              << "  P               : MODO TORMENTA (lluvia, luna, faro, mar bravo)\n"
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