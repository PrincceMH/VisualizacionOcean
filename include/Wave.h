#ifndef WAVE_H
#define WAVE_H

class Wave {
private:
    float amplitude;
    float frequency;
    float direction;
    float phase;

public:
    // Constructor
    Wave(float amp, float freq, float dir, float ph);

    // Métodos de acceso (Getters) para la sumatoria matemática
    float getAmplitude() const { return amplitude; }
    float getFrequency() const { return frequency; }
    float getDirection() const { return direction; }
    float getPhase() const { return phase; }

    // Metodo para calcular el número de onda (k)
    float getWaveNumber() const;
};

#endif