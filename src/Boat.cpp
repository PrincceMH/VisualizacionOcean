#include "../include/Boat.h"
#include "../include/Ocean.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

static const float PI = 3.14159265358979323846f;

namespace {
    struct V3 { float x, y, z; };

    V3 sub(const V3& a, const V3& b) { return V3{ a.x - b.x, a.y - b.y, a.z - b.z }; }

    V3 crossV(const V3& a, const V3& b) {
        return V3{ a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x };
    }

    V3 normalizeV(V3 v) {
        float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
        if (len > 1e-6f) { v.x /= len; v.y /= len; v.z /= len; }
        return v;
    }

    // Emite un triangulo con su normal de cara (sombreado plano, tipo "low poly")
    void emitTri(const V3& a, const V3& b, const V3& c) {
        V3 n = normalizeV(crossV(sub(b, a), sub(c, a)));
        glNormal3f(n.x, n.y, n.z);
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
    }

    // Emite un quad (A,B,C,D en orden) como dos triangulos, con normal de cara
    void emitQuad(const V3& a, const V3& b, const V3& c, const V3& d) {
        emitTri(a, b, c);
        emitTri(a, c, d);
    }

    // --- Version con textura, usada solo para la cubierta ---
    // Genera coordenadas UV a partir de la posicion local (x,z) del vertice,
    // escaladas por uScale/vScale (ajusta estos valores para que el patron
    // de la textura se vea del tamaño correcto sobre la cubierta).
    void emitTriTex(const V3& a, const V3& b, const V3& c, float uScale, float vScale) {
        V3 n = normalizeV(crossV(sub(b, a), sub(c, a)));
        glNormal3f(n.x, n.y, n.z);
        glTexCoord2f(a.x * uScale, a.z * vScale); glVertex3f(a.x, a.y, a.z);
        glTexCoord2f(b.x * uScale, b.z * vScale); glVertex3f(b.x, b.y, b.z);
        glTexCoord2f(c.x * uScale, c.z * vScale); glVertex3f(c.x, c.y, c.z);
    }

    void emitQuadTex(const V3& a, const V3& b, const V3& c, const V3& d, float uScale, float vScale) {
        emitTriTex(a, b, c, uScale, vScale);
        emitTriTex(a, c, d, uScale, vScale);
    }
}

Boat::Boat(float posX, float posZ, float len, float wid, float hgt)
    : x(posX), z(posZ), length(len), width(wid), hullHeight(hgt) {}

void Boat::draw(const Ocean& ocean, float time, GLuint texCubierta) const {
    // Altura del agua bajo el centro, la proa y la popa, para inclinar
    // el barco (pitch) segun la pendiente de la ola bajo el casco
    float centerY = ocean.getHeightAt(x, z, time);
    float frontY  = ocean.getHeightAt(x, z + length * 0.5f, time);
    float backY   = ocean.getHeightAt(x, z - length * 0.5f, time);

    float pitchRad = atan2f(frontY - backY, length);
    float pitchDeg = pitchRad * 180.0f / PI;

    glPushMatrix();
        glTranslatef(x, centerY + hullHeight * 0.35f, z);
        glRotatef(-pitchDeg, 1.0f, 0.0f, 0.0f);

        float hw = width * 0.5f;
        float hh = hullHeight * 0.5f;
        float hl = length * 0.5f;

        // -----------------------------------------------------------
        // CASCO: definido por "estaciones" (secciones transversales) a
        // lo largo del eje Z, de popa (-hl) a proa (+hl). Cada estacion
        // tiene su propio ancho de borda y profundidad de quilla, para
        // lograr un casco en V curvo (no una caja recta) con la proa
        // levantada, como en la foto de referencia.
        // -----------------------------------------------------------
        struct Station { float z, hw, deckY, keelY; };
        Station st[] = {
            { -hl,          0.05f * hw, hh * 0.75f, -hh * 0.35f }, // popa (espejo, casi en punta)
            { -hl * 0.55f,  0.85f * hw, hh * 0.85f, -hh * 0.85f },
            {  0.0f,        1.00f * hw, hh * 0.90f, -hh * 1.00f }, // manga maxima
            {  hl * 0.45f,  0.75f * hw, hh * 1.05f, -hh * 0.70f },
            {  hl * 0.80f,  0.35f * hw, hh * 1.30f, -hh * 0.40f },
            {  hl,          0.02f * hw, hh * 1.55f, -hh * 0.10f }, // proa en punta y levantada
        };
        const int nSt = 6;

        // Paneles de casco (izquierdo y derecho), cada uno un triangulo
        // desde la borda hasta la quilla central
        glColor3f(0.10f, 0.18f, 0.36f); // azul oscuro, como en la foto
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < nSt - 1; ++i) {
            const Station& a = st[i];
            const Station& b = st[i + 1];

            V3 railA_L = { -a.hw, a.deckY, a.z };
            V3 railB_L = { -b.hw, b.deckY, b.z };
            V3 keelA   = { 0.0f,  a.keelY, a.z };
            V3 keelB   = { 0.0f,  b.keelY, b.z };

            emitTri(railA_L, railB_L, keelB);
            emitTri(railA_L, keelB, keelA);

            V3 railA_R = { a.hw, a.deckY, a.z };
            V3 railB_R = { b.hw, b.deckY, b.z };

            emitTri(keelA, keelB, railB_R);
            emitTri(keelA, railB_R, railA_R);
        }
        glEnd();

        // ==================================================
        // Cubierta (plano superior entre las dos bordas) - CON TEXTURA
        // ==================================================
        const float deckUScale = 0.9f; // repeticiones de la textura a lo ancho
        const float deckVScale = 0.5f; // repeticiones de la textura a lo largo

        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBindTexture(GL_TEXTURE_2D, texCubierta);
        glColor3f(1.0f, 1.0f, 1.0f); // color real, sin tintar la textura

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < nSt - 1; ++i) {
            const Station& a = st[i];
            const Station& b = st[i + 1];
            V3 aL = { -a.hw, a.deckY, a.z };
            V3 bL = { -b.hw, b.deckY, b.z };
            V3 bR = {  b.hw, b.deckY, b.z };
            V3 aR = {  a.hw, a.deckY, a.z };
            emitQuadTex(aL, bL, bR, aR, deckUScale, deckVScale);
        }
        glEnd();

        glDisable(GL_TEXTURE_2D); // el resto del barco (cabina, mastil) no lleva textura


        float cabinZ0 = -hl * 0.10f, cabinZ1 = hl * 0.35f;
        float cabinW  = hw * 1.15f;
        float cabinY0 = hh * 0.95f, cabinY1 = hh * 2.3f;
        float roofY   = hh * 2.7f;

        glColor3f(0.85f, 0.85f, 0.82f);
        glBegin(GL_TRIANGLES);
            // frente
            emitQuad({-cabinW, cabinY0, cabinZ1}, {cabinW, cabinY0, cabinZ1},
                      {cabinW, cabinY1, cabinZ1}, {-cabinW, cabinY1, cabinZ1});
            // atras
            emitQuad({cabinW, cabinY0, cabinZ0}, {-cabinW, cabinY0, cabinZ0},
                      {-cabinW, cabinY1, cabinZ0}, {cabinW, cabinY1, cabinZ0});
            // izquierda
            emitQuad({-cabinW, cabinY0, cabinZ0}, {-cabinW, cabinY0, cabinZ1},
                      {-cabinW, cabinY1, cabinZ1}, {-cabinW, cabinY1, cabinZ0});
            // derecha
            emitQuad({cabinW, cabinY0, cabinZ1}, {cabinW, cabinY0, cabinZ0},
                      {cabinW, cabinY1, cabinZ0}, {cabinW, cabinY1, cabinZ1});
        glEnd();

        // Techo inclinado hacia atras (un solo plano)
        glColor3f(0.35f, 0.35f, 0.38f);
        glBegin(GL_TRIANGLES);
            emitQuad({-cabinW, cabinY1, cabinZ1}, {cabinW, cabinY1, cabinZ1},
                      {cabinW, roofY, cabinZ0}, {-cabinW, roofY, cabinZ0});
        glEnd();

        // Ventana frontal (panel oscuro sobre la pared del frente)
        glColor3f(0.10f, 0.14f, 0.18f);
        glBegin(GL_TRIANGLES);
            float winY0 = cabinY0 + (cabinY1 - cabinY0) * 0.35f;
            float winY1 = cabinY0 + (cabinY1 - cabinY0) * 0.85f;
            emitQuad({-cabinW * 0.75f, winY0, cabinZ1 + 0.01f},
                      { cabinW * 0.75f, winY0, cabinZ1 + 0.01f},
                      { cabinW * 0.75f, winY1, cabinZ1 + 0.01f},
                      {-cabinW * 0.75f, winY1, cabinZ1 + 0.01f});
        glEnd();


        float mastX = 0.0f, mastZ = cabinZ0 - 0.05f;
        float mastBaseY = roofY;
        float mastTopY  = roofY + hullHeight * 3.2f;

        glColor3f(0.55f, 0.55f, 0.55f);
        glPushMatrix();
            glTranslatef(mastX, mastBaseY, mastZ);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            GLUquadric* mast = gluNewQuadric();
            gluCylinder(mast, 0.05, 0.035, mastTopY - mastBaseY, 8, 1);
            gluDeleteQuadric(mast);
        glPopMatrix();

        // Barra transversal (spreader) de donde salen los botalones
        float spreaderY = mastBaseY + (mastTopY - mastBaseY) * 0.35f;
        glPushMatrix();
            glTranslatef(-hw * 1.3f, spreaderY, mastZ);
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            GLUquadric* bar = gluNewQuadric();
            gluCylinder(bar, 0.03, 0.03, hw * 2.6f, 6, 1);
            gluDeleteQuadric(bar);
        glPopMatrix();

        // Botalones angulados hacia arriba y hacia los costados
        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
                glTranslatef(mastX, spreaderY, mastZ);
                glRotatef(side * 55.0f, 0.0f, 0.0f, 1.0f);
                glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
                GLUquadric* boom = gluNewQuadric();
                gluCylinder(boom, 0.035, 0.015, hullHeight * 3.6f, 6, 1);
                gluDeleteQuadric(boom);
            glPopMatrix();
        }

        // Antena / remate rojo en la punta del mastil
        glColor3f(0.7f, 0.15f, 0.15f);
        glPushMatrix();
            glTranslatef(mastX, mastTopY, mastZ);
            glutSolidSphere(0.06, 8, 8);
        glPopMatrix();

    glPopMatrix();
}