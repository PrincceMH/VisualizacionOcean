#include "../include/Lighthouse.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

Lighthouse::Lighthouse(float posX, float posY, float posZ, float towerH, float baseR, float topR)
    : x(posX), y(posY), z(posZ), baseRadius(baseR), topRadius(topR), towerHeight(towerH) {}

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