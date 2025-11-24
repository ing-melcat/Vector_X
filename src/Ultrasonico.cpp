#include "Ultrasonico.h"

Ultrasonico::Ultrasonico(int trig, int echo) {
    this->trigPin = trig;
    this->echoPin = echo;
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
}

float Ultrasonico::leer_distancia() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    
    // Cálculo de distancia en cm
    return duration * 0.0343 / 2;
}