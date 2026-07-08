#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "../include/Ocean.h"

// Creamos una instancia global de nuestro océano
// 50 filas, 50 columnas, con una separación de 0.5 entre cada punto

Ocean miOceano(50, 50, 0.5f);

// Configura una fuente de luz (ambiental + difusa + especular) y el
// material del oceano (con especular fuerte para simular brillos en las olas)
void initLuzYMaterial() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // por seguridad, garantiza normales unitarias

    // --- Fuente de luz (GL_LIGHT0) ---
    GLfloat luzAmbiental[]  = { 0.25f, 0.25f, 0.3f, 1.0f };
    GLfloat luzDifusa[]     = { 0.8f,  0.8f,  0.8f, 1.0f };
    GLfloat luzEspecular[]  = { 1.0f,  1.0f,  1.0f, 1.0f };
    GLfloat posicionLuz[]   = { 10.0f, 15.0f, 10.0f, 1.0f }; // luz posicional

    glLightfv(GL_LIGHT0, GL_AMBIENT,  luzAmbiental);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, posicionLuz);

    // --- Material del oceano ---
    // GL_COLOR_MATERIAL deja que glColor3f() (en display()) defina el
    // componente ambiental y difuso del material, para poder seguir
    // controlando el color del agua facilmente.
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // El componente especular y el brillo (shininess) se fijan aparte:
    // esto es lo que genera los reflejos/brillos sobre las olas.
    GLfloat matEspecular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT, GL_SPECULAR, matEspecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 96.0f); // mas alto = brillo mas pequeno y concentrado
}


void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    
    gluLookAt(0.0, 10.0, 20.0,  
              0.0, 0.0, 0.0,   
              0.0, 1.0, 0.0);

    
    glColor3f(0.0f, 0.8f, 1.0f);

    miOceano.draw();

    glutSwapBuffers();
}


// Avanza el tiempo de simulacion y solicita un nuevo cuadro.
void idle() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    miOceano.update(time);
    glutPostRedisplay();
}

// Cierra la ventana con ESC o 'q'.
void keyboard(unsigned char key, int x, int y) {
    if (key == 27 || key == 'q' || key == 'Q') {
        exit(0);
    }
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = w * 1.0f / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
   
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    glutCreateWindow("Proyecto Oceano");

    glEnable(GL_DEPTH_TEST);

    initLuzYMaterial();

    miOceano.loadWaves("data/spectrum.txt");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    
    return 0;
}