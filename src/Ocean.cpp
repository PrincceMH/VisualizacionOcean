#include "../include/Ocean.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h" 

static const float PI = 3.14159265358979323846f;

//dimensiones y malla
Ocean::Ocean(int numRings, int numSectors, float radius)
    : rows(numRings), cols(numSectors + 1), maxRadius(radius),
      textureID(0), showTexture(true), maxHeight(1.0f), foamPass(false) {
    initMesh();
}

// Malla en disco (coordenadas polares): anillos concentricos (i = radio) y
// sectores angulares (j = angulo). El borde queda a distancia constante del
// centro, asi la niebla lo funde de forma uniforme y el mar parece infinito.
void Ocean::initMesh() {
    mesh.resize(rows, std::vector<WPoint>(cols));

    const float uvTile = 18.0f;   // cada 18 unidades del mundo se repite la textura

    for (int i = 0; i < rows; ++i) {
        // radio del anillo: i=0 -> centro (r=0), i=rows-1 -> borde (r=maxRadius)
        float r = (rows > 1) ? ((float)i / (rows - 1)) * maxRadius : 0.0f;

        for (int j = 0; j < cols; ++j) {
            // angulo del sector: en j=cols-1 vale 2*PI == 0, cerrando el circulo
            float theta = ((float)j / (cols - 1)) * 2.0f * PI;

            mesh[i][j].x = r * cosf(theta);
            mesh[i][j].y = 0.0f;
            mesh[i][j].z = r * sinf(theta);

            // Normal inicial hacia arriba
            mesh[i][j].nx = 0.0f;
            mesh[i][j].ny = 1.0f;
            mesh[i][j].nz = 0.0f;

            // UV planar sobre el plano del disco: la textura mosaico se repite en
            // el mundo (evita el remolino/pellizco que darian unas UV polares).
            mesh[i][j].s = mesh[i][j].x / uvTile;
            mesh[i][j].t = mesh[i][j].z / uvTile;
        }
    }
}

// Cantidad de espuma segun la altura del vertice (0 = sin espuma, 1 = espuma total).
// Se normaliza contra la cresta mas alta del frame, asi el tramo superior de las
// olas siempre recibe espuma sin importar la escala del oleaje.
float Ocean::foamFactor(float y) const {
    float ini = 0.45f * maxHeight;             // altura donde empieza la espuma
    float fin = 0.80f * maxHeight;             // altura de espuma total (solo crestas)
    float foam = (y - ini) / (fin - ini);
    if (foam < 0.0f) foam = 0.0f;
    if (foam > 1.0f) foam = 1.0f;
    return foam;
}

// Envia un vertice. En la pasada de agua usa el color azul del oceano; en la
// pasada de espuma usa un gris = cantidad de espuma (que se sumara sobre el agua).
void Ocean::emitVertex(const WPoint& p) {
    if (foamPass) {
        float f = foamFactor(p.y);
        glColor3f(f, f, f);                    // negro en valles (suma 0), blanco en crestas
    } else {
        glColor3f(0.26f, 0.52f, 0.70f);        // azul del agua
    }

    glNormal3f(p.nx, p.ny, p.nz);
    glTexCoord2f(p.s, p.t);
    glVertex3f(p.x, p.y, p.z);
}

// Emite los triangulos de la malla (posicion, normal, texcoord y color por vertice)
void Ocean::drawTriangles() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            const WPoint& p00 = mesh[i][j];
            const WPoint& p10 = mesh[i + 1][j];
            const WPoint& p01 = mesh[i][j + 1];
            const WPoint& p11 = mesh[i + 1][j + 1];

            if (i == 0) {
                // El anillo 0 colapsa en un unico centro. Se emite un solo
                // triangulo por sector para evitar la cara degenerada.
                emitVertex(p00);
                emitVertex(p11);
                emitVertex(p10);
            } else {
                // Orden antihorario visto desde arriba: normales hacia +Y.
                emitVertex(p00);
                emitVertex(p01);
                emitVertex(p10);

                emitVertex(p01);
                emitVertex(p11);
                emitVertex(p10);
            }
        }
    }
    glEnd();
}

// Dibuja la superficie rellena afectada por la iluminacion y material.
void Ocean::draw() {

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // --- Pasada 1: agua texturizada ---
    if (showTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    foamPass = false;
    drawTriangles();
    if (showTexture) glDisable(GL_TEXTURE_2D);

    // --- Pasada 2: espuma sumada sobre las crestas (blending aditivo) ---
    // El color destino recibe (color_agua + espuma). Los valles suman 0 (negro)
    // y las crestas suman blanco, por lo que la espuma resalta sobre la textura.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);       // aditivo: destino + fuente
    glDepthFunc(GL_LEQUAL);            // misma geometria: se permite profundidad igual
    glDepthMask(GL_FALSE);            // la espuma no reescribe el buffer de profundidad
    foamPass = true;
    drawTriangles();
    foamPass = false;
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);

    glDisable(GL_COLOR_MATERIAL);
}

// Lee amplitud, direccion y frecuencia por linea desde el archivo de espectro;
bool Ocean::loadWaves(const std::string& filename) {
    
    std::vector<std::string> posiblesRutas = {
        filename,
        "./" + filename,
        "../" + filename,
        "../../" + filename,
        "./data/spectrum.txt",
        "../data/spectrum.txt",
        "data/spectrum.txt"
    };

    std::ifstream file;
    std::string rutaUsada;
    for (const std::string& ruta : posiblesRutas) {
        file.open(ruta.c_str());
        if (file.is_open()) {
            rutaUsada = ruta;
            break;
        }
        file.clear();
    }

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo de espectro: " << filename
                   << " (revisa que este en la misma carpeta desde donde "
                   << "se ejecuta el programa)" << std::endl;
        std::cerr << "Se usaran olas por defecto para que la simulacion no quede plana." << std::endl;

        waves.clear();
        waves.push_back(Wave(0.5f, 0.15f, 30.0f  * PI / 180.0f, 0.0f));
        waves.push_back(Wave(0.3f, 0.25f, 90.0f  * PI / 180.0f, 60.0f  * PI / 180.0f));
        waves.push_back(Wave(0.2f, 0.40f, 150.0f * PI / 180.0f, 120.0f * PI / 180.0f));
        return false;
    }

    waves.clear();
    std::string linea;
    while (std::getline(file, linea)) {
        // Se ignoran lineas vacias y comentarios (las que empiezan con #)
        size_t inicio = linea.find_first_not_of(" \t\r");
        if (inicio == std::string::npos || linea[inicio] == '#') continue;

        std::istringstream ss(linea);
        float amplitude, direction, frequency;
        if (ss >> amplitude >> direction >> frequency) {
            float phase = static_cast<float>(rand()) / RAND_MAX * 2.0f * PI;
            waves.push_back(Wave(amplitude, frequency, direction, phase));
        }
    }
    file.close();

    std::cout << "Se cargaron " << waves.size() << " olas desde " << rutaUsada << std::endl;
    return !waves.empty();
}

bool Ocean::loadTexture(const std::string& filename) {
    // 1. Generar un ID para la textura en OpenGL
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 2. Configurar cómo se repetira la textura (ideal para el oceano)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // 3. Configurar el filtrado para evitar que se vea muy pixeleado
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 4. Cargar la imagen con stb_image
    int width, height, nrChannels;
    // TGA suele tener la imagen invertida verticalmente, esto lo corrige:
    stbi_set_flip_vertically_on_load(true); 
    
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        // Determinar si la imagen tiene canal Alfa (transparencia) o solo RGB
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
        std::cout << "Textura cargada exitosamente: " << filename << std::endl;
        return true;
    } else {
        std::cerr << "Error al cargar la textura: " << filename << std::endl;
        return false;
    }
}

void Ocean::update(float time) {
    float maxH = 0.001f;   // se rastrea la cresta mas alta de este frame

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            float x = mesh[i][j].x;
            float z = mesh[i][j].z;
            float height = 0.0f;

            for (size_t w = 0; w < waves.size(); ++w) {
                const Wave& wave = waves[w];
                float k = wave.getWaveNumber();
                float theta = k * (x * cosf(wave.getDirection()) + z * sinf(wave.getDirection()))
                              - 2.0f * PI * wave.getFrequency() * time
                              + wave.getPhase();
                height += wave.getAmplitude() * cosf(theta);
            }

            mesh[i][j].y = height;
            if (height > maxH) maxH = height;
        }
    }

    maxHeight = maxH;
    computeNormals();
}

<<<<<<< HEAD
=======
// Evalua h(x,z,t) igual que update(), pero para un punto cualquiera
// (no tiene que ser un vertice de la malla)
>>>>>>> integracion
float Ocean::getHeightAt(float x, float z, float time) const {
    float height = 0.0f;
    for (size_t w = 0; w < waves.size(); ++w) {
        const Wave& wave = waves[w];
        float k = wave.getWaveNumber();
        float theta = k * (x * cosf(wave.getDirection()) + z * sinf(wave.getDirection()))
                      - 2.0f * PI * wave.getFrequency() * time
                      + wave.getPhase();
        height += wave.getAmplitude() * cosf(theta);
    }
    return height;
}

<<<<<<< HEAD

=======
>>>>>>> integracion
// Calcula la normal de cada vertice promediando las normales de las caras
// (triangulos) que lo tocan
void Ocean::computeNormals() {
    // 1. Reiniciar el acumulador de cada vertice
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mesh[i][j].nx = 0.0f;
            mesh[i][j].ny = 0.0f;
            mesh[i][j].nz = 0.0f;
        }
    }

    // 2. Recorrer cada celda de la malla (2 triangulos por celda) y
    //    acumular la normal de cara en sus 3 vertices
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            WPoint& p00 = mesh[i][j];
            WPoint& p10 = mesh[i + 1][j];
            WPoint& p01 = mesh[i][j + 1];
            WPoint& p11 = mesh[i + 1][j + 1];

            // Primer triangulo con orden antihorario visto desde +Y.
            // En el centro se usa p00-p11-p10 y se omite la cara degenerada.
            WPoint& firstA = p00;
            WPoint& firstB = (i == 0) ? p11 : p01;
            WPoint& firstC = p10;

            float e1x = firstB.x - firstA.x, e1y = firstB.y - firstA.y, e1z = firstB.z - firstA.z;
            float e2x = firstC.x - firstA.x, e2y = firstC.y - firstA.y, e2z = firstC.z - firstA.z;

            float fnx = e1y * e2z - e1z * e2y;
            float fny = e1z * e2x - e1x * e2z;
            float fnz = e1x * e2y - e1y * e2x;

            firstA.nx += fnx; firstA.ny += fny; firstA.nz += fnz;
            firstB.nx += fnx; firstB.ny += fny; firstB.nz += fnz;
            firstC.nx += fnx; firstC.ny += fny; firstC.nz += fnz;

            if (i == 0) continue;

            // Segundo triangulo: p01-p11-p10.
            e1x = p11.x - p01.x; e1y = p11.y - p01.y; e1z = p11.z - p01.z;
            e2x = p10.x - p01.x; e2y = p10.y - p01.y; e2z = p10.z - p01.z;

            fnx = e1y * e2z - e1z * e2y;
            fny = e1z * e2x - e1x * e2z;
            fnz = e1x * e2y - e1y * e2x;

            p01.nx += fnx; p01.ny += fny; p01.nz += fnz;
            p10.nx += fnx; p10.ny += fny; p10.nz += fnz;
            p11.nx += fnx; p11.ny += fny; p11.nz += fnz;
        }
    }

    // 2b. Costura: los vertices j=0 y j=cols-1 son el MISMO punto del disco
    //     (angulo 0 y 2*PI). Cada uno acumulo normales de un solo lado, asi que
    //     se suman para que la normal sea continua y no se vea una linea.
    for (int i = 0; i < rows; ++i) {
        WPoint& a = mesh[i][0];
        WPoint& b = mesh[i][cols - 1];
        float sx = a.nx + b.nx, sy = a.ny + b.ny, sz = a.nz + b.nz;
        a.nx = b.nx = sx;
        a.ny = b.ny = sy;
        a.nz = b.nz = sz;
    }

    // 2c. Centro: todos los vertices del anillo 0 estan en el origen. Se
    //     promedian sus normales y se asigna una unica normal al centro.
    {
        float cx = 0.0f, cy = 0.0f, cz = 0.0f;
        for (int j = 0; j < cols - 1; ++j) {   // se omite el duplicado de costura
            cx += mesh[0][j].nx; cy += mesh[0][j].ny; cz += mesh[0][j].nz;
        }
        for (int j = 0; j < cols; ++j) {
            mesh[0][j].nx = cx; mesh[0][j].ny = cy; mesh[0][j].nz = cz;
        }
    }

    // 3. Normalizar el promedio acumulado en cada vertice
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            WPoint& p = mesh[i][j];
            float len = sqrtf(p.nx * p.nx + p.ny * p.ny + p.nz * p.nz);
            if (len > 0.00001f) {
                p.nx /= len;
                p.ny /= len;
                p.nz /= len;
            } else {
                p.nx = 0.0f; p.ny = 1.0f; p.nz = 0.0f;
            }
        }
    }
}
