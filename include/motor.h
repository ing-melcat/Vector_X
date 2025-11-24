#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>

class Motor {
    private:
        int pos_izq, neg_izq;
        int pos_der, neg_der;
        int pin_ena, pin_enb;
        int velocidad_actual;

        // Método privado para aplicar PWM
        void aplicarPotencia(int potencia);

    public:
        // Constructor
        Motor(int pi, int ni, int pd, int nd, int ena, int enb, int velocidad);
        
        // Control de movimiento
        void avanzar();
        void detener();
        void derecha();
        void izquierda();
        void retroceder();
        
        // Ajuste dinámico de velocidad
        void setVelocidad(int nueva_velocidad);
};

#endif