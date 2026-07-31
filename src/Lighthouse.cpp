#include "../include/Lighthouse.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Lighthouse::Lighthouse(float posX, float posY, float posZ, float towerH, float baseR, float topR)
    : x(posX), y(posY), z(posZ), baseRadius(baseR), topRadius(topR), towerHeight(towerH) {}

// Luz PUNTUAL calida del faro con GL_LIGHT1 (w=1): tiene posicion en la linterna
// y atenuacion con la distancia. Da un glow al entorno; el "rayo" visible que
// gira lo dibuja drawBeam(). Debe fijarse con la camara ya aplicada.
void Lighthouse::applyLight(float intensity) const {
    glEnable(GL_LIGHT1);

    // Posicion en la linterna. w=1 -> luz puntual (no direccional como el sol).
    GLfloat pos[] = { x, y + towerHeight * 1.15f, z, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    // Color calido escalado por la intensidad (tenue de dia, fuerte en tormenta).
    GLfloat warm[] = { 1.0f * intensity, 0.85f * intensity, 0.45f * intensity, 1.0f };
    GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT,  none);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  warm);
    glLightfv(GL_LIGHT1, GL_SPECULAR, warm);

    // Atenuacion: propia de una luz puntual, decae con la distancia
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.03f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.006f);

    // Luz puntual (sin cono): da un glow calido a la torre y su entorno, sin el
    // borde duro que causaba la mancha. El "rayo" visible lo dibuja drawBeam().
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 180.0f);
}

// Haz de luz VISIBLE del faro: un cono translucido que GIRA (barre como un faro)
// pero con la inclinacion fija hacia la ALTURA de la camara, de modo que en cada
// vuelta, al pasar por tu direccion, el rayo te apunta de lleno. Se dibuja con
// blending aditivo. Debe llamarse despues de la geometria opaca, con la camara.
void Lighthouse::drawBeam(float time, float camX, float camY, float camZ, float intensity) const {
    if (intensity <= 0.01f) return;
    const float PI = 3.14159265f;
    float baseY = y + towerHeight * 1.15f;   // sale de la linterna

    // Elevacion hacia la camara: el haz barre a esa altura, asi al pasar por tu
    // azimut te apunta. La distancia fija el largo para que el rayo te alcance.
    float dx = camX - x, dy = camY - baseY, dz = camZ - z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    float h    = sqrtf(dx * dx + dz * dz);
    float elev = (h > 0.001f) ? atan2f(dy, h) * 180.0f / PI : 0.0f;

    float len = (dist > 1.0f) ? dist : 42.0f;   // el haz llega hasta la camara
    float rad = 4.5f;                            // radio del cono (un poco mayor)
    float angleDeg = time * 45.0f;               // barrido (grados por unidad de tiempo)

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);        // aditivo: el haz SUMA luz (glow)
    glDepthMask(GL_FALSE);                     // el haz no tapa la geometria

    glPushMatrix();
        glTranslatef(x, baseY, z);
        glRotatef(angleDeg, 0.0f, 1.0f, 0.0f); // barrido en azimut (gira con el tiempo)
        glRotatef(elev,     0.0f, 0.0f, 1.0f); // inclinacion fija hacia la altura de la camara

        glBegin(GL_TRIANGLES);
        const int seg = 20;
        for (int i = 0; i < seg; ++i) {
            float t0 = 2.0f * PI * i / seg;
            float t1 = 2.0f * PI * (i + 1) / seg;
            float y0 = rad * cosf(t0), z0 = rad * sinf(t0);
            float y1 = rad * cosf(t1), z1 = rad * sinf(t1);

            glColor4f(1.0f, 0.90f, 0.55f, 0.35f * intensity);  // apice (linterna): brillante
            glVertex3f(0.0f, 0.0f, 0.0f);
            glColor4f(1.0f, 0.85f, 0.45f, 0.0f);   // extremo: transparente
            glVertex3f(len, y0, z0);
            glVertex3f(len, y1, z1);
        }
        glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

void Lighthouse::draw(GLuint textureID) const {
    glPushMatrix();
        glTranslatef(x, y, z);

        // Encendemos el motor 2D y amarramos la textura "faro.jpg"
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        const int nBands = 6;
        float bandHeight = towerHeight / nBands;
        for (int i = 0; i < nBands; ++i) {
            float r0 = baseRadius + (topRadius - baseRadius) * ((float)i / nBands);
            float r1 = baseRadius + (topRadius - baseRadius) * ((float)(i + 1) / nBands);

            // Mantenemos la alternancia de color para teñir la textura
            if (i % 2 == 0) glColor3f(0.88f, 0.88f, 0.85f); // banda blanca
            else             glColor3f(0.75f, 0.15f, 0.12f); // banda roja

            glPushMatrix();
                glTranslatef(0.0f, i * bandHeight, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                GLUquadric* q = gluNewQuadric();
                gluQuadricNormals(q, GLU_SMOOTH);
                
                // Activar coordenadas de textura para envolver el cilindro
                gluQuadricTexture(q, GL_TRUE); 
                
                gluCylinder(q, r0, r1, bandHeight, 16, 1);
                gluDeleteQuadric(q);
            glPopMatrix();
        }

        // Apagamos las texturas para pintar los detalles superiores con colores solidos
        glDisable(GL_TEXTURE_2D);

        float galleryY = towerHeight;
        float galleryR = topRadius * 1.6f;
        glColor3f(0.25f, 0.25f, 0.25f);
        glPushMatrix();
            glTranslatef(0.0f, galleryY, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            GLUquadric* qg = gluNewQuadric();
            gluCylinder(qg, galleryR, galleryR, towerHeight * 0.05f, 16, 1);
            gluDeleteQuadric(qg);
        glPopMatrix();

        float lanternH = towerHeight * 0.25f;
        float lanternR = topRadius * 1.1f;
        float lanternBaseY = galleryY + towerHeight * 0.05f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.75f, 0.85f, 0.95f, 0.55f);
        glPushMatrix();
            glTranslatef(0.0f, lanternBaseY, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            GLUquadric* ql = gluNewQuadric();
            gluCylinder(ql, lanternR, lanternR, lanternH, 12, 1);
            gluDeleteQuadric(ql);
        glPopMatrix();
        glDisable(GL_BLEND);

        // Techo conico de la linterna
        glColor3f(0.20f, 0.20f, 0.20f);
        glPushMatrix();
            glTranslatef(0.0f, lanternBaseY + lanternH, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            GLUquadric* qr = gluNewQuadric();
            gluCylinder(qr, lanternR, 0.0f, lanternR * 1.3f, 12, 1);
            gluDeleteQuadric(qr);
        glPopMatrix();

        // Foco / destello en el centro de la linterna
        glColor3f(1.0f, 0.95f, 0.6f);
        glPushMatrix();
            glTranslatef(0.0f, lanternBaseY + lanternH * 0.5f, 0.0f);
            glutSolidSphere(lanternR * 0.35, 10, 10);
        glPopMatrix();

    glPopMatrix();
}