#pragma once // O las guardas que ya tengas escritas

// --- AGREGA ESTE BLOQUE ---
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
// --------------------------

class Lighthouse {
private:
    float x, y, z;
    float baseRadius;
    float topRadius;
    float towerHeight;

public:
    Lighthouse(float posX, float posY, float posZ, float towerH, float baseR, float topR);

    // Ahora el compilador ya sabrá qué es GLuint
    void draw(GLuint textureID) const;

    // Configura GL_LIGHT1 como luz puntual del faro, con intensidad 0..1
    // (tenue de dia, fuerte en tormenta). Debe llamarse tras gluLookAt.
    void applyLight(float intensity) const;

    // Dibuja el haz de luz visible (cono translucido giratorio, inclinado hacia
    // la camara), con intensidad 0..1. Debe llamarse despues de la geometria opaca.
    void drawBeam(float time, float camX, float camY, float camZ, float intensity) const;
};
