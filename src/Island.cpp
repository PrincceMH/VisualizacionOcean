#include "../include/Island.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>

static const float PI = 3.14159265358979323846f;

Island::Island(float cx, float cz, float radX, float radZ, float maxH,
               int ringsCount, int slicesCount, float irregularityAmount, int seed)
    : rings(ringsCount), slices(slicesCount), radiusX(radX), radiusZ(radZ),
      maxHeight(maxH), centerX(cx), centerZ(cz),
      irregularity(irregularityAmount), noiseSeed(seed) {
    buildMesh();
}

float Island::shapeFactor(float ang) const {
    return 1.0f
        + irregularity * 0.6f  * sinf(ang * 2.0f + noiseSeed * 0.7f)
        + irregularity * 0.4f  * sinf(ang * 5.0f + noiseSeed * 1.3f)
        + irregularity * 0.25f * sinf(ang * 9.0f + noiseSeed * 2.1f);
}

void Island::buildMesh() {
    mesh.resize(rings + 1, std::vector<IPoint>(slices + 1));

    for (int r = 0; r <= rings; ++r) {
        float t = (float)r / (float)rings;          // 0 = centro, 1 = borde
        float h = maxHeight * cosf(t * (PI / 2.0f)); // maximo al centro, 0 en el borde

        for (int s = 0; s <= slices; ++s) {
            float ang = 2.0f * PI * (float)s / (float)slices;
            float shape = shapeFactor(ang);

            IPoint& p = mesh[r][s];
            p.x = centerX + t * radiusX * shape * cosf(ang);
            p.y = h;
            p.z = centerZ + t * radiusZ * shape * sinf(ang);
            p.nx = 0.0f; p.ny = 1.0f; p.nz = 0.0f;
        }
    }

    computeNormals();
}

// Misma tecnica que Ocean::computeNormals: normal de cada vertice como el
// promedio (sin normalizar antes) de las normales de las caras vecinas.
void Island::computeNormals() {
    for (int r = 0; r <= rings; ++r) {
        for (int s = 0; s <= slices; ++s) {
            mesh[r][s].nx = mesh[r][s].ny = mesh[r][s].nz = 0.0f;
        }
    }

    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < slices; ++s) {
            IPoint& p00 = mesh[r][s];
            IPoint& p10 = mesh[r + 1][s];
            IPoint& p01 = mesh[r][s + 1];
            IPoint& p11 = mesh[r + 1][s + 1];

            float e1x = p10.x - p00.x, e1y = p10.y - p00.y, e1z = p10.z - p00.z;
            float e2x = p01.x - p00.x, e2y = p01.y - p00.y, e2z = p01.z - p00.z;
            float fnx = e1y * e2z - e1z * e2y;
            float fny = e1z * e2x - e1x * e2z;
            float fnz = e1x * e2y - e1y * e2x;
            p00.nx += fnx; p00.ny += fny; p00.nz += fnz;
            p10.nx += fnx; p10.ny += fny; p10.nz += fnz;
            p01.nx += fnx; p01.ny += fny; p01.nz += fnz;

            e1x = p10.x - p01.x; e1y = p10.y - p01.y; e1z = p10.z - p01.z;
            e2x = p11.x - p01.x; e2y = p11.y - p01.y; e2z = p11.z - p01.z;
            fnx = e1y * e2z - e1z * e2y;
            fny = e1z * e2x - e1x * e2z;
            fnz = e1x * e2y - e1y * e2x;
            p01.nx += fnx; p01.ny += fny; p01.nz += fnz;
            p10.nx += fnx; p10.ny += fny; p10.nz += fnz;
            p11.nx += fnx; p11.ny += fny; p11.nz += fnz;
        }
    }

    for (int r = 0; r <= rings; ++r) {
        for (int s = 0; s <= slices; ++s) {
            IPoint& p = mesh[r][s];
            float len = sqrtf(p.nx * p.nx + p.ny * p.ny + p.nz * p.nz);
            if (len > 0.00001f) {
                p.nx /= len; p.ny /= len; p.nz /= len;
            } else {
                p.nx = 0.0f; p.ny = 1.0f; p.nz = 0.0f;
            }
        }
    }
}

// Color aproximado segun la altura relativa: arena cerca de la orilla,
// vegetacion en la parte media, roca hacia la cima.
void Island::colorForHeight(float t) const {
    if (t < 0.12f) {
        glColor3f(0.80f, 0.72f, 0.52f); // arena
    } else if (t < 0.55f) {
        glColor3f(0.30f, 0.50f, 0.20f); // vegetacion
    } else {
        glColor3f(0.42f, 0.37f, 0.32f); // roca
    }
}


void Island::drawRock(float cx, float cy, float cz, float scale, int seed) const {
    auto noise = [seed](int i) {
        float v = sinf((float)seed * 12.9898f + (float)i * 78.233f) * 43758.5453f;
        return v - floorf(v);
    };

    float topY = cy + scale * (0.55f + 0.30f * noise(0));
    float botY = cy - scale * (0.35f + 0.20f * noise(1));
    float midY = cy + scale * (0.10f * noise(2) - 0.05f);
    float midR = scale * (0.55f + 0.35f * noise(3));

    struct V { float x, y, z; };
    V top = { cx, topY, cz };
    V bot = { cx, botY, cz };
    V mid[4];
    for (int i = 0; i < 4; ++i) {
        float ang = (PI / 2.0f) * i + noise(4 + i) * 0.6f;
        float r = midR * (0.75f + 0.5f * noise(8 + i));
        mid[i].x = cx + cosf(ang) * r;
        mid[i].y = midY + (noise(12 + i) - 0.5f) * scale * 0.3f;
        mid[i].z = cz + sinf(ang) * r;
    }

    auto emitTri = [](V a, V b, V c) {
        float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        float nx = uy * vz - uz * vy, ny = uz * vx - ux * vz, nz = ux * vy - uy * vx;
        float len = sqrtf(nx * nx + ny * ny + nz * nz);
        if (len > 1e-5f) { nx /= len; ny /= len; nz /= len; }
        glNormal3f(nx, ny, nz);
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
    };

    glColor3f(0.45f, 0.44f, 0.42f); // gris roca
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; ++i) {
        V& a = mid[i];
        V& b = mid[(i + 1) % 4];
        emitTri(top, a, b);
        emitTri(bot, b, a);
    }
    glEnd();
}

void Island::draw() const {
    glBegin(GL_TRIANGLES);
    for (int r = 0; r < rings; ++r) {
        for (int s = 0; s < slices; ++s) {
            const IPoint& p00 = mesh[r][s];
            const IPoint& p10 = mesh[r + 1][s];
            const IPoint& p01 = mesh[r][s + 1];
            const IPoint& p11 = mesh[r + 1][s + 1];

            colorForHeight(p00.y / maxHeight);
            glNormal3f(p00.nx, p00.ny, p00.nz);
            glVertex3f(p00.x, p00.y, p00.z);

            colorForHeight(p10.y / maxHeight);
            glNormal3f(p10.nx, p10.ny, p10.nz);
            glVertex3f(p10.x, p10.y, p10.z);

            colorForHeight(p01.y / maxHeight);
            glNormal3f(p01.nx, p01.ny, p01.nz);
            glVertex3f(p01.x, p01.y, p01.z);

            colorForHeight(p01.y / maxHeight);
            glNormal3f(p01.nx, p01.ny, p01.nz);
            glVertex3f(p01.x, p01.y, p01.z);

            colorForHeight(p10.y / maxHeight);
            glNormal3f(p10.nx, p10.ny, p10.nz);
            glVertex3f(p10.x, p10.y, p10.z);

            colorForHeight(p11.y / maxHeight);
            glNormal3f(p11.nx, p11.ny, p11.nz);
            glVertex3f(p11.x, p11.y, p11.z);
        }
    }
    glEnd();

    // "Falda" submarina: baja desde el borde real de la isla (que ya
    // incluye la forma irregular) hasta bien por debajo del nivel del mar,
    // para que nunca se vea un hueco entre el agua y la base de la isla
    const float skirtDepth = 4.0f;
    glColor3f(0.32f, 0.30f, 0.28f);
    glBegin(GL_TRIANGLES);
    for (int s = 0; s < slices; ++s) {
        const IPoint& e0 = mesh[rings][s];
        const IPoint& e1 = mesh[rings][s + 1];

        float top0x = e0.x, top0z = e0.z;
        float top1x = e1.x, top1z = e1.z;
        float botY = -skirtDepth;

        float midx = (top0x + top1x) * 0.5f - centerX;
        float midz = (top0z + top1z) * 0.5f - centerZ;
        float len = sqrtf(midx * midx + midz * midz);
        if (len > 1e-5f) { midx /= len; midz /= len; }
        glNormal3f(midx, 0.0f, midz);

        glVertex3f(top0x, 0.0f, top0z);
        glVertex3f(top0x, botY, top0z);
        glVertex3f(top1x, botY, top1z);

        glNormal3f(midx, 0.0f, midz);
        glVertex3f(top0x, 0.0f, top0z);
        glVertex3f(top1x, botY, top1z);
        glVertex3f(top1x, 0.0f, top1z);
    }
    glEnd();

    // Rocas alrededor de la orilla, siguiendo el mismo contorno irregular
    // que el borde real de la isla (no un circulo aparte)
    const int nRocks = 16;
    for (int i = 0; i < nRocks; ++i) {
        float t = (float)i / (float)nRocks;
        float ang = 2.0f * PI * t + 0.35f;
        float shape = shapeFactor(ang) * (0.95f + 0.10f * sinf((float)i * 3.1f));
        float rx = centerX + radiusX * shape * cosf(ang);
        float rz = centerZ + radiusZ * shape * sinf(ang);
        float avgRadius = (radiusX + radiusZ) * 0.5f;
        float scale = avgRadius * (0.06f + 0.03f * fabsf(sinf((float)i * 1.7f)));
        drawRock(rx, 0.0f, rz, scale, i * 37 + 5);
    }
}
