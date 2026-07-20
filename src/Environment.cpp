#include "../include/Environment.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

static const float PI = 3.14159265f;

Environment::Environment()
    : sunX(0.55f), sunY(0.90f), sunSize(0.19f),
      sunDirX(-0.35f), sunDirY(0.85f), sunDirZ(-0.45f) {}

void Environment::initLight() const {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    // El brillo especular se calcula con la posicion real de la camara: el
    // reflejo del sol se concentra y se mueve con las olas (destello, no mancha).
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    // El especular se aplica DESPUES de la textura, no antes: asi el brillo
    // no se ensucia al modularse con ocean.tga y se mantiene limpio y blanco.
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    GLfloat ambient[]  = { 0.44f, 0.47f, 0.54f, 1.0f };
    GLfloat diffuse[]  = { 0.62f, 0.60f, 0.52f, 1.0f };
    GLfloat specular[] = { 0.45f, 0.44f, 0.40f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    applyLight();
}

void Environment::applyLight() const {
    GLfloat sunDir[] = { sunDirX, sunDirY, sunDirZ, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}

void Environment::drawSky(int w, int h) const {
    float horizon = h * 0.48f;

    glBegin(GL_QUADS);
        glColor3f(0.58f, 0.50f, 0.57f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f((float)w, 0.0f);
        glColor3f(0.88f, 0.75f, 0.57f);
        glVertex2f((float)w, horizon);
        glVertex2f(0.0f, horizon);

        glColor3f(0.88f, 0.75f, 0.57f);
        glVertex2f(0.0f, horizon);
        glVertex2f((float)w, horizon);
        glColor3f(0.73f, 0.90f, 0.91f);
        glVertex2f((float)w, (float)h);
        glVertex2f(0.0f, (float)h);
    glEnd();
}

void Environment::drawSun(int w, int h) const {
    float cx = w * sunX;
    float cy = h * sunY;
    float r = h * sunSize;
    float core = r * 0.43f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.99f, 0.86f, 1.0f);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 64; ++i) {
            float a = 2.0f * PI * i / 64.0f;
            glColor4f(1.0f, 0.95f, 0.72f, 0.82f);
            glVertex2f(cx + cosf(a) * core, cy + sinf(a) * core);
        }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= 64; ++i) {
            float a = 2.0f * PI * i / 64.0f;
            float x = cosf(a);
            float y = sinf(a);
            glColor4f(1.0f, 0.95f, 0.72f, 0.82f);
            glVertex2f(cx + x * core, cy + y * core);
            glColor4f(1.0f, 0.82f, 0.46f, 0.0f);
            glVertex2f(cx + x * r, cy + y * r);
        }
    glEnd();

    glDisable(GL_BLEND);
}

void Environment::draw(int w, int h) const {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, w, 0.0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    drawSky(w, h);
    drawSun(w, h);

    glDepthMask(GL_TRUE);
    glPopAttrib();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}



