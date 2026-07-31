#ifndef LIGHTHOUSE_H
#define LIGHTHOUSE_H

// Representa un faro: torre troncoconica con franjas rojo/blanco, una
// galeria (balcon) bajo la linterna, la linterna (cuarto de luz, con
// "vidrio" translucido) y un techo conico rematando la punta.
class Lighthouse {
private:
    float x, y, z;       // base del faro (y = altura del terreno donde se apoya)
    float baseRadius;
    float topRadius;
    float towerHeight;

public:
    Lighthouse(float posX, float posY, float posZ,
               float towerH = 4.0f, float baseR = 0.55f, float topR = 0.35f);

    void draw() const;

    // Configura GL_LIGHT1 como luz puntual del faro. Debe llamarse con la camara
    // ya aplicada (justo despues de gluLookAt) y antes de dibujar.
    void applyLight(float time) const;

    // Dibuja el haz de luz visible (cono translucido giratorio). Debe llamarse
    // despues de la geometria opaca de la escena.
    void drawBeam(float time) const;
};

#endif
