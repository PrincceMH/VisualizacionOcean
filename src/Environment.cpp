#include "../include/Environment.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

static const float PI = 3.14159265f;
static const float HALF_PI = PI * 0.5f;

namespace {

float clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

float mixValue(float from, float to, float amount) {
    return from + (to - from) * amount;
}

void setSkyColor(float x, float y, float z,
                 float toSunX, float toSunZ) {
    const float horizon[3] = { 0.62f, 0.30f, 0.39f };
    const float zenith[3]  = { 0.10f, 0.17f, 0.38f };
    const float nadir[3]   = { 0.17f, 0.13f, 0.24f };
    const float sunset[3]  = { 1.00f, 0.36f, 0.10f };

    float r;
    float g;
    float b;

    if (y >= 0.0f) {
        const float height = powf(clamp01(y), 0.62f);
        r = mixValue(horizon[0], zenith[0], height);
        g = mixValue(horizon[1], zenith[1], height);
        b = mixValue(horizon[2], zenith[2], height);
    } else {
        const float depth = clamp01(-y * 1.8f);
        r = mixValue(horizon[0], nadir[0], depth);
        g = mixValue(horizon[1], nadir[1], depth);
        b = mixValue(horizon[2], nadir[2], depth);
    }

    const float horizontalLength = sqrtf(x * x + z * z);
    const float sunHorizontalLength =
        sqrtf(toSunX * toSunX + toSunZ * toSunZ);

    float alignment = 0.0f;
    if (horizontalLength > 0.0001f && sunHorizontalLength > 0.0001f) {
        alignment = (x * toSunX + z * toSunZ) /
                    (horizontalLength * sunHorizontalLength);
        alignment = clamp01(alignment);
    }

    const float horizonBand = 1.0f - clamp01(fabsf(y) / 0.58f);
    const float nearSun = powf(alignment, 5.0f);
    const float warmth = horizonBand * (0.08f + 0.58f * nearSun);

    r = mixValue(r, sunset[0], warmth);
    g = mixValue(g, sunset[1], warmth);
    b = mixValue(b, sunset[2], warmth);

    glColor3f(r, g, b);
}

} // namespace

Environment::Environment()
    : toSunX(-0.34f), toSunY(0.18f), toSunZ(-0.92f),
      skyRadius(80.0f), sunCoreRadius(2.35f), sunGlowRadius(7.5f) {
    const float length = sqrtf(toSunX * toSunX +
                               toSunY * toSunY +
                               toSunZ * toSunZ);
    if (length > 0.0001f) {
        toSunX /= length;
        toSunY /= length;
        toSunZ /= length;
    }
}

void Environment::initLight() const {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT2);
    glEnable(GL_NORMALIZE);

    // La luz ambiental representa la contribucion difusa del cielo completo;
    // evita que las superficies que no miran al sol se vuelvan negras.
    const GLfloat globalAmbient[] = { 0.17f, 0.19f, 0.27f, 1.0f };

    // GL_LIGHT0 es exclusivamente la luz principal calida del atardecer.
    const GLfloat sunAmbient[]  = { 0.05f, 0.045f, 0.04f, 1.0f };
    const GLfloat sunDiffuse[]  = { 1.00f, 0.62f, 0.38f, 1.0f };
    const GLfloat sunSpecular[] = { 1.00f, 0.78f, 0.56f, 1.0f };

    // GL_LIGHT2 aproxima la iluminacion hemisferica fria de la boveda. No
    // aporta especular: el brillo definido sigue perteneciendo al sol.
    const GLfloat skyAmbient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    const GLfloat skyDiffuse[]  = { 0.28f, 0.36f, 0.52f, 1.0f };
    const GLfloat noSpecular[]  = { 0.0f, 0.0f, 0.0f, 1.0f };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    // El especular se suma despues de modular la textura, para conservar un
    // reflejo solar limpio sin teñirlo con ocean.tga.
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    glLightfv(GL_LIGHT2, GL_AMBIENT, skyAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, skyDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, noSpecular);
}

void Environment::applyLight() const {
    // OpenGL transforma GL_POSITION por la matriz de vista actual. Por eso esta
    // llamada debe hacerse cada frame, inmediatamente despues de gluLookAt.
    const GLfloat toSun[] = { toSunX, toSunY, toSunZ, 0.0f };
    const GLfloat fromSky[] = { 0.18f, 0.94f, 0.28f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, toSun);
    glLightfv(GL_LIGHT2, GL_POSITION, fromSky);
}

void Environment::drawSky() const {
    const int latitudeBands = 20;
    const int longitudeBands = 48;

    for (int latitude = 0; latitude < latitudeBands; ++latitude) {
        const float lat0 = -HALF_PI + PI * latitude / latitudeBands;
        const float lat1 = -HALF_PI + PI * (latitude + 1) / latitudeBands;
        const float y0 = sinf(lat0);
        const float y1 = sinf(lat1);
        const float ring0 = cosf(lat0);
        const float ring1 = cosf(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int longitude = 0; longitude <= longitudeBands; ++longitude) {
            const float angle = 2.0f * PI * longitude / longitudeBands;
            const float sinAngle = sinf(angle);
            const float cosAngle = cosf(angle);

            const float x0 = ring0 * sinAngle;
            const float z0 = ring0 * cosAngle;
            setSkyColor(x0, y0, z0, toSunX, toSunZ);
            glVertex3f(x0 * skyRadius, y0 * skyRadius, z0 * skyRadius);

            const float x1 = ring1 * sinAngle;
            const float z1 = ring1 * cosAngle;
            setSkyColor(x1, y1, z1, toSunX, toSunZ);
            glVertex3f(x1 * skyRadius, y1 * skyRadius, z1 * skyRadius);
        }
        glEnd();
    }
}

void Environment::drawSun() const {
    const float distance = skyRadius * 0.88f;
    const float centerX = toSunX * distance;
    const float centerY = toSunY * distance;
    const float centerZ = toSunZ * distance;

    // Base ortonormal del disco. Al ser perpendicular a toSun, el sol siempre
    // mira a la camara situada en el centro de la boveda.
    float rightX = -toSunZ;
    float rightY = 0.0f;
    float rightZ = toSunX;
    const float rightLength = sqrtf(rightX * rightX + rightZ * rightZ);
    if (rightLength > 0.0001f) {
        rightX /= rightLength;
        rightZ /= rightLength;
    }

    const float upX = rightY * toSunZ - rightZ * toSunY;
    const float upY = rightZ * toSunX - rightX * toSunZ;
    const float upZ = rightX * toSunY - rightY * toSunX;
    const int segments = 64;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);

    // Halo aditivo suave.
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.52f, 0.20f, 0.38f);
        glVertex3f(centerX, centerY, centerZ);
        for (int i = 0; i <= segments; ++i) {
            const float angle = 2.0f * PI * i / segments;
            const float dx = (rightX * cosf(angle) + upX * sinf(angle)) *
                             sunGlowRadius;
            const float dy = (rightY * cosf(angle) + upY * sinf(angle)) *
                             sunGlowRadius;
            const float dz = (rightZ * cosf(angle) + upZ * sinf(angle)) *
                             sunGlowRadius;
            glColor4f(1.0f, 0.35f, 0.08f, 0.0f);
            glVertex3f(centerX + dx, centerY + dy, centerZ + dz);
        }
    glEnd();

    // Nucleo definido con mezcla alfa para conservar un borde limpio.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
        glColor4f(1.0f, 0.98f, 0.78f, 1.0f);
        glVertex3f(centerX, centerY, centerZ);
        for (int i = 0; i <= segments; ++i) {
            const float angle = 2.0f * PI * i / segments;
            const float dx = (rightX * cosf(angle) + upX * sinf(angle)) *
                             sunCoreRadius;
            const float dy = (rightY * cosf(angle) + upY * sinf(angle)) *
                             sunCoreRadius;
            const float dz = (rightZ * cosf(angle) + upZ * sinf(angle)) *
                             sunCoreRadius;
            glColor4f(1.0f, 0.68f, 0.28f, 1.0f);
            glVertex3f(centerX + dx, centerY + dy, centerZ + dz);
        }
    glEnd();
}

void Environment::draw(float cameraX, float cameraY, float cameraZ) const {
    GLint previousMatrixMode = GL_MODELVIEW;
    glGetIntegerv(GL_MATRIX_MODE, &previousMatrixMode);

    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT |
                 GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glShadeModel(GL_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(cameraX, cameraY, cameraZ);

    drawSky();
    drawSun();

    glPopMatrix();

    glPopAttrib();
    glMatrixMode(previousMatrixMode);
}



