#include "../include/Lighthouse.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Lighthouse::Lighthouse(float posX, float posY, float posZ, float towerH, float baseR, float topR)
    : x(posX), y(posY), z(posZ), baseRadius(baseR), topRadius(topR), towerHeight(towerH) {}

// Foco giratorio del faro con GL_LIGHT1. Es una luz PUNTUAL (w=1) convertida en
// reflector (spotlight): tiene posicion, atenuacion con la distancia, un cono
// (SPOT_CUTOFF) y una direccion que gira con el tiempo, barriendo el horizonte.
void Lighthouse::applyLight(float time) const {
    glEnable(GL_LIGHT1);

    // Posicion en la linterna. w=1 -> luz puntual (no direccional como el sol).
    GLfloat pos[] = { x, y + towerHeight * 1.15f, z, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    // Color calido del haz
    GLfloat warm[] = { 1.0f, 0.85f, 0.45f, 1.0f };
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
    (void)time;
}

// Haz de luz VISIBLE del faro: un cono translucido que gira, dibujado como
// geometria con blending aditivo (no es iluminacion, es el "rayo" que se ve
// cruzar el aire, como en un faro real). Brillante en la linterna, se desvanece
// hacia el extremo. Debe llamarse despues de la geometria opaca.
void Lighthouse::drawBeam(float time) const {
    const float PI = 3.14159265f;
    float baseY = y + towerHeight * 1.15f;   // sale de la linterna
    float len   = 42.0f;                      // largo del haz
    float rad   = 3.2f;                       // radio del cono en el extremo
    float tilt  = -0.10f;                     // leve inclinacion hacia el mar
    float angleDeg = time * 45.0f;            // barrido (grados por unidad de tiempo)

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);        // aditivo: el haz SUMA luz (glow)
    glDepthMask(GL_FALSE);                     // el haz no tapa la geometria

    glPushMatrix();
        glTranslatef(x, baseY, z);
        glRotatef(angleDeg, 0.0f, 1.0f, 0.0f); // gira el haz alrededor del eje Y

        glBegin(GL_TRIANGLES);
        const int seg = 20;
        for (int i = 0; i < seg; ++i) {
            float t0 = 2.0f * PI * i / seg;
            float t1 = 2.0f * PI * (i + 1) / seg;
            float y0 = tilt * len + rad * cosf(t0), z0 = rad * sinf(t0);
            float y1 = tilt * len + rad * cosf(t1), z1 = rad * sinf(t1);

            glColor4f(1.0f, 0.90f, 0.55f, 0.35f);  // apice (linterna): brillante
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

void Lighthouse::draw() const {
    glPushMatrix();
        glTranslatef(x, y, z);

        const int nBands = 6;
        float bandHeight = towerHeight / nBands;
        for (int i = 0; i < nBands; ++i) {
            float r0 = baseRadius + (topRadius - baseRadius) * ((float)i / nBands);
            float r1 = baseRadius + (topRadius - baseRadius) * ((float)(i + 1) / nBands);

            if (i % 2 == 0) glColor3f(0.88f, 0.88f, 0.85f); // banda blanca
            else             glColor3f(0.75f, 0.15f, 0.12f); // banda roja

            glPushMatrix();
                glTranslatef(0.0f, i * bandHeight, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                GLUquadric* q = gluNewQuadric();
                gluQuadricNormals(q, GLU_SMOOTH);
                gluCylinder(q, r0, r1, bandHeight, 16, 1);
                gluDeleteQuadric(q);
            glPopMatrix();
        }

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

        //  Techo conico de la linterna
        glColor3f(0.20f, 0.20f, 0.20f);
        glPushMatrix();
            glTranslatef(0.0f, lanternBaseY + lanternH, 0.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            GLUquadric* qr = gluNewQuadric();
            gluCylinder(qr, lanternR, 0.0f, lanternR * 1.3f, 12, 1);
            gluDeleteQuadric(qr);
        glPopMatrix();

        //  Foco / destello en el centro de la linterna
        glColor3f(1.0f, 0.95f, 0.6f);
        glPushMatrix();
            glTranslatef(0.0f, lanternBaseY + lanternH * 0.5f, 0.0f);
            glutSolidSphere(lanternR * 0.35, 10, 10);
        glPopMatrix();

    glPopMatrix();
}
