## Compilación y ejecución

### macOS

```bash
g++ -std=c++11 src/main.cpp src/Ocean.cpp src/Wave.cpp -o Oceano -framework OpenGL -framework GLUT -Wno-deprecated-declarations
./Oceano
```

### Windows (MinGW + freeglut)

```bash
g++ src/main.cpp src/Ocean.cpp src/Wave.cpp -o Oceano.exe -lopengl32 -lglu32 -lfreeglut
Oceano.exe
```