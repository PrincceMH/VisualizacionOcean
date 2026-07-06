#include "../include/Wave.h"

//inicializa los valores de la ola
Wave::Wave(float amp, float freq, float dir, float ph) 
    : amplitude(amp), frequency(freq), direction(dir), phase(ph) {}

// Metodo para calcular el número de onda (k)
float Wave::getWaveNumber() const {
    const float PI = 3.14159265358979323846f;
    return (4.0f * PI * PI * frequency * frequency) / 9.81f;
}