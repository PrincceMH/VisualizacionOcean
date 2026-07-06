#include <GL/glut.h>
#include "../include/Ocean.h"

// Creamos una instancia global de nuestro océano
// 50 filas, 50 columnas, con una separación de 0.5 entre cada punto

Ocean miOceano(100, 100, 0.5f);


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

// Se ejecuta continuamente cuando GLUT no tiene otros eventos que atender.
// Es lo que realmente hace que el oceano se anime frame a frame.
void idle() {
    float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // tiempo en segundos
    miOceano.update(t);
    glutPostRedisplay();
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

    // Carga las olas desde el archivo de texto (amplitud freq direccion fase)
    miOceano.loadWaves("waves.txt");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutMainLoop();

    return 0;
}
