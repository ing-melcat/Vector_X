#ifndef ULTRASONICO_H
#define ULTRASONICO_H
#include <Arduino.h>

class Ultrasonico {
    private:
        int trigPin;
        int echoPin;
    public:
        Ultrasonico(int trig, int echo);
        float leer_distancia();
};

#endif