#pragma once
#include "Sprite.h"
#include <iostream>

// Tipos de objetos
enum PickupType {
    PICKUP_HEALTH,  // Botiqu�n
    PICKUP_AMMO,    // Munici�n
    PICKUP_WEAPON   // Arma
};

class Pickup : public Sprite {
public:
    PickupType type;
    int amount; // Cu�nto cura o cu�ntas balas da
	float floatingOffset = 0.0f; // Flotaci�n
    // Constructor
    Pickup(float x, float y, SDL_Surface* tex, PickupType t, int val)
        : Sprite(x, y, tex, scale) // Llamamos al constructor del padre
    {
        type = t;
        amount = val;

		// Los pickups son m�s peque�os
        scale = 0.4f;
    }

    // Sobreescribimos para identificarlo f�cil
    bool isPickup() override { return true; }

    // Funci�n que se ejecuta al tocarlo
    void onCollect(int& playerHP, int& playerAmmo) {
        switch (type) {
        case PICKUP_HEALTH:
            playerHP += amount;
			if (playerHP > 100) playerHP = 100; // Maximo 100 de vida
            std::cout << "Recogido Botiquin. Vida: " << playerHP << std::endl;
            break;

        case PICKUP_AMMO:
            playerAmmo += amount;
            std::cout << "Recogida Municion. Total: " << playerAmmo << std::endl;
            break;
        }

		// Objeto como "muerto" para borrar
        state = STATE_DEAD;
        isDead = true;
    }

    void update(float deltaTime) override {
		// Llamar al update base para futuras animaciones
        Sprite::update(deltaTime);
        
        float time = SDL_GetTicks() * 0.003f;
        float phase = (x + y); // Seed aleatoria basada en posici�n
        
		zOffset = sin(time + phase) * 30.0f; // Oscilaci�n vertical
        
    }
};