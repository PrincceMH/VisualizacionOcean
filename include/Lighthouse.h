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
};