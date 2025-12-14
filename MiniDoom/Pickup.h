#pragma once
#include "Sprite.h"
#include <iostream>

// Tipos de objetos
enum PickupType {
    PICKUP_HEALTH,  // Botiquín
    PICKUP_AMMO,    // Munición
    PICKUP_WEAPON   // Arma
};

class Pickup : public Sprite {
public:
    PickupType type;
    int amount; // Cuánto cura o cuántas balas da
	float floatingOffset = 0.0f; // Para efecto de flotación
    // Constructor
    Pickup(float x, float y, SDL_Surface* tex, PickupType t, int val)
        : Sprite(x, y, tex, scale) // Llamamos al constructor del padre
    {
        type = t;
        amount = val;

        // Ajuste visual: Los items suelen ser más pequeños que los monstruos
        // Usamos la variable 'scale' que añadimos antes
        scale = 0.4f;

        // Los items a veces flotan un poco más abajo (para que parezcan estar en el suelo)
        // Puedes ajustar esto en tu Sprite::draw si tienes soporte para vMove
    }

    // Sobreescribimos para identificarlo fácil
    bool isPickup() override { return true; }

    // Función que se ejecuta al tocarlo
    void onCollect(int& playerHP, int& playerAmmo) {
        switch (type) {
        case PICKUP_HEALTH:
            playerHP += amount;
            if (playerHP > 100) playerHP = 100; // Tope de vida
            std::cout << "Recogido Botiquin. Vida: " << playerHP << std::endl;
            break;

        case PICKUP_AMMO:
            playerAmmo += amount;
            std::cout << "Recogida Municion. Total: " << playerAmmo << std::endl;
            break;
        }

        // Marcamos el objeto como "muerto" para que el juego lo borre
        state = STATE_DEAD;
        isDead = true; // Asumiendo que tienes una flag para borrar
    }

    void update(float deltaTime) override {
        // 1. Llamar al update base (por si añades animaciones de sprites luego)
        Sprite::update(deltaTime);

        // 2. MAGIA MATEMÁTICA: Onda Senoidal
        // SDL_GetTicks() * 0.005f -> Controla la VELOCIDAD
        // * 15.0f -> Controla la ALTURA (Amplitud)
        
        // Truco Pro: Sumamos (x + y) * 100 al tiempo.
        // Esto hace que cada objeto flote en un tiempo distinto (Desfase).
        // Si no haces esto, todos los objetos del mapa subirán y bajarán perfectamente sincronizados (se ve robótico).
        
        float time = SDL_GetTicks() * 0.003f;
        float phase = (x + y); // Semilla aleatoria basada en posición
        
        zOffset = sin(time + phase) * 30.0f; 
        
        // Nota: 20.0f es un valor en píxeles de pantalla aproximados. 
        // Ajústalo si flotan muy alto o muy bajo.
    }
};