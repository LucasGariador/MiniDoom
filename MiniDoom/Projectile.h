#pragma once
#include "Sprite.h"

struct Projectile
{
    float x, y;          // Posición en el mapa
    float dx, dy;        // Velocidad (dirección * velocidad)
    float speed = 10.0f; // Velocidad de la bola de fuego
    bool active = true;  // ¿Sigue volando?
    bool hostile; // TRUE = Te daña a ti. FALSE = Daña enemigos.
    // Reutilizamos tu clase Sprite para dibujarlo
    Sprite* spriteVis;

    Projectile(float startX, float startY, float angle, SDL_Surface* texture, bool isHostile) {
        x = startX;
        y = startY;
        hostile = isHostile;
        // Calculamos cuánto se mueve en X e Y basándonos en el ángulo
        dx = cosf(angle) * speed;
        dy = sinf(angle) * speed;

        // Creamos un Sprite visual para este proyectil
        spriteVis = new Sprite(x, y, texture, 0.5f);
    }

    ~Projectile() {
        // Nota: No borramos la textura aquí porque es compartida
        delete spriteVis;
    }

    void update(float deltaTime, int mapW, int mapH, int worldMap[][30]) {
        if (!active) return;

        // 1. Mover
        x += dx * deltaTime;
        y += dy * deltaTime;

        // 2. Actualizar la posición del sprite visual para que se dibuje bien
        spriteVis->x = x;
        spriteVis->y = y;

        // 3. Choque con Paredes
        // Convertimos posición float a int para ver la celda del mapa
        int mapX = (int)x;
        int mapY = (int)y;

        // Si se sale del mapa o choca con pared > 0
        if (mapX < 0 || mapX >= mapW || mapY < 0 || mapY >= mapH || worldMap[mapY][mapX] > 0) {
            active = false; // Destruir proyectil
            // Aquí podrías reproducir sonido de impacto
        }
    }
};

