#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>

class Motor {
    private:
        int pos_izq, neg_izq;
        int pos_der, neg_der;
        int pin_ena, pin_enb;
        int velocidad_actual;

        // --- CONFIGURACIÓN PWM ---
        // CAMBIO CRÍTICO: Usamos canales altos (12 y 13).
        // Estos canales usan los Timers de "Baja Velocidad" del ESP32.
        // Así NO tocan el Timer 0 que usa la cámara para el XCLK.
        const int frecuencia = 1000; 
        const int resolucion = 8;    
        const int canal_a = 12;       // Canal seguro 1
        const int canal_b = 13;       // Canal seguro 2

        void aplicarPotencia(int potencia);

    public:
        Motor(int pi, int ni, int pd, int nd, int ena, int enb, int velocidad);
        
        void avanzar();
        void detener();
        void derecha();
        void izquierda();
        void retroceder();
        
        void setVelocidad(int nueva_velocidad);
};

#endif