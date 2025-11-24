#include "motor.h"

Motor::Motor(int pi, int ni, int pd, int nd, int ena, int enb, int velocidad) {
    this->pos_izq = pi;
    this->neg_izq = ni;
    this->pos_der = pd;
    this->neg_der = nd;
    this->pin_ena = ena;
    this->pin_enb = enb;
    this->velocidad_actual = velocidad;

    pinMode(pos_izq, OUTPUT);
    pinMode(neg_izq, OUTPUT);
    pinMode(pos_der, OUTPUT);
    pinMode(neg_der, OUTPUT);
    pinMode(pin_ena, OUTPUT);
    pinMode(pin_enb, OUTPUT);

    detener(); 
}

void Motor::aplicarPotencia(int potencia) {
    analogWrite(pin_ena, potencia);
    analogWrite(pin_enb, potencia);
}

void Motor::setVelocidad(int nueva_velocidad) {
    this->velocidad_actual = nueva_velocidad;
}

void Motor::avanzar() {
    aplicarPotencia(velocidad_actual);
    digitalWrite(pos_izq, HIGH);
    digitalWrite(neg_izq, LOW);
    digitalWrite(pos_der, HIGH);
    digitalWrite(neg_der, LOW);
}

void Motor::detener() {
    aplicarPotencia(0);
    digitalWrite(pos_izq, LOW);
    digitalWrite(neg_izq, LOW);
    digitalWrite(pos_der, LOW);
    digitalWrite(neg_der, LOW);
}

void Motor::retroceder() {
    aplicarPotencia(velocidad_actual);
    digitalWrite(pos_izq, LOW);
    digitalWrite(neg_izq, HIGH);
    digitalWrite(pos_der, LOW);
    digitalWrite(neg_der, HIGH);
}

void Motor::derecha() {
    aplicarPotencia(velocidad_actual);
    digitalWrite(pos_izq, HIGH);
    digitalWrite(neg_izq, LOW);
    digitalWrite(pos_der, LOW);
    digitalWrite(neg_der, HIGH);
}

void Motor::izquierda() {
    aplicarPotencia(velocidad_actual);
    digitalWrite(pos_izq, LOW);
    digitalWrite(neg_izq, HIGH);
    digitalWrite(pos_der, HIGH);
    digitalWrite(neg_der, LOW);
}