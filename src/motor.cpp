#include "motor.h"

Motor::Motor(int pi, int ni, int pd, int nd, int ena, int enb, int velocidad) {
    this->pos_izq = pi;
    this->neg_izq = ni;
    this->pos_der = pd;
    this->neg_der = nd;
    this->pin_ena = ena;
    this->pin_enb = enb;
    this->velocidad_actual = velocidad;

    // Configurar pines de dirección
    pinMode(pos_izq, OUTPUT);
    pinMode(neg_izq, OUTPUT);
    pinMode(pos_der, OUTPUT);
    pinMode(neg_der, OUTPUT);

    // --- CONFIGURACIÓN PWM SEGURA ---
    // Usamos canales 12 y 13 (Low Speed) para no matar la cámara
    ledcSetup(canal_a, frecuencia, resolucion);
    ledcSetup(canal_b, frecuencia, resolucion);

    // Adjuntar canales a los pines físicos
    ledcAttachPin(pin_ena, canal_a);
    ledcAttachPin(pin_enb, canal_b);

    detener(); 
}

void Motor::aplicarPotencia(int potencia) {
    int p = constrain(potencia, 0, 255);
    ledcWrite(canal_a, p);
    ledcWrite(canal_b, p);
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