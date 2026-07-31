#include "../include/Rain.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>

namespace {
    const float GRAVITY      = 30.0f;   // aceleracion de caida
    const float TERMINAL_VEL = 45.0f;   // velocidad maxima de caida
    const float DROP_W       = 0.10f;   // medio ancho de la gota
    const float DROP_H       = 0.55f;   // medio alto (mayor: parece raya de lluvia)

    float frand() { return rand() / (float)RAND_MAX; }
    float frand(float a, float b) { return a + frand() * (b - a); }
}

Rain::Rain(int count) : texID(0), area(45.0f), topOffset(55.0f) {
    drops.resize(count);
    // Se reparten a alturas aleatorias para que la lluvia no caiga "en bloque"
    for (Drop& d : drops) {
        d.x = frand(-area, area);
        d.z = frand(-area, area);
        d.y = frand(0.0f, topOffset);
        d.vy = frand(0.0f, 10.0f);
    }
}

// Recoloca una gota arriba, dentro de la columna de lluvia centrada en la camara.
void Rain::respawn(Drop& d, float camX, float camY, float camZ) const {
    d.x = camX + frand(-area, area);
    d.z = camZ + frand(-area, area);
    d.y = camY + frand(topOffset * 0.5f, topOffset);
    d.vy = frand(0.0f, 10.0f);
}

// Textura procedural de la gota: un ovalo claro que se desvanece en los bordes
// (alfa alto al centro, 0 en las esquinas), igual idea que el lab de billboards.
void Rain::initTexture() {
    const int S = 32;
    static unsigned char px[S * S * 4];
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            float u = (x + 0.5f) / S * 2.0f - 1.0f;
            float v = (y + 0.5f) / S * 2.0f - 1.0f;
            float d = sqrtf(u * u + v * v);
            float alpha = 1.0f - d;
            if (alpha < 0.0f) alpha = 0.0f;
            alpha = alpha * alpha;   // concentrado en el centro

            int i = (y * S + x) * 4;
            px[i + 0] = 205; px[i + 1] = 225; px[i + 2] = 255;   // azul-blanco
            px[i + 3] = (unsigned char)(alpha * 255);
        }
    }

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, S, S, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
}

void Rain::update(float dt, float camX, float camY, float camZ) {
    for (Drop& d : drops) {
        d.vy = fminf(d.vy + GRAVITY * dt, TERMINAL_VEL);
        d.y -= d.vy * dt;

        // Reaparece si toca el mar o si la camara se alejo de la gota
        float dx = d.x - camX, dz = d.z - camZ;
        if (d.y < camY - topOffset || (dx * dx + dz * dz) > (area * 1.6f) * (area * 1.6f)) {
            respawn(d, camX, camY, camZ);
        }
    }
}

void Rain::draw(float intensity) const {
    if (intensity <= 0.01f) return;

    // Ejes derecha/arriba de la camara, leidos de la matriz de vista: cada quad
    // se construye con ellos para encarar siempre al observador (billboard).
    GLfloat m[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, m);
    float rx = m[0], ry = m[4], rz = m[8];    // eje derecha
    float ux = m[1], uy = m[5], uz = m[9];    // eje arriba

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);                    // la lluvia no reescribe profundidad

    glColor4f(0.80f, 0.86f, 1.0f, 0.55f * intensity);

    glBegin(GL_QUADS);
    for (const Drop& d : drops) {
        float ax = rx * DROP_W, ay = ry * DROP_W, az = rz * DROP_W;   // desplazamiento horizontal
        float bx = ux * DROP_H, by = uy * DROP_H, bz = uz * DROP_H;   // desplazamiento vertical

        glTexCoord2f(0, 0); glVertex3f(d.x - ax - bx, d.y - ay - by, d.z - az - bz);
        glTexCoord2f(1, 0); glVertex3f(d.x + ax - bx, d.y + ay - by, d.z + az - bz);
        glTexCoord2f(1, 1); glVertex3f(d.x + ax + bx, d.y + ay + by, d.z + az + bz);
        glTexCoord2f(0, 1); glVertex3f(d.x - ax + bx, d.y - ay + by, d.z - az + bz);
    }
    glEnd();

    glDepthMask(GL_TRUE);
    glPopAttrib();
}
