#pragma once
#include "Sprite.h"

struct Projectile
{
    float x, y;
    float dx, dy;
    float speed = 10.0f;
    bool active = true;
    bool hostile;

    Sprite* spriteVis;

    Projectile(float startX, float startY, float angle, SDL_Surface* texture, bool isHostile) {
        x = startX;
        y = startY;
        hostile = isHostile;

        dx = cosf(angle) * speed;
        dy = sinf(angle) * speed;

        // Creamos un Sprite visual para este proyectil
        spriteVis = new Sprite(x, y, texture, 3.0f);
    }

    ~Projectile() {
        delete spriteVis;
    }

    void update(float deltaTime, int mapW, int mapH, const std::vector<std::vector<int>>& worldMap) {
        if (!active) return;

        // 1. Mover
        x += dx * deltaTime;
        y += dy * deltaTime;

        // 2. Actualizar la posicion del sprite visual para que se dibuje bien
        spriteVis->x = x;
        spriteVis->y = y;

        // 3. Choque con Paredes
        // Convertimos posici�n float a int para ver la celda del mapa
        int mapX = (int)x;
        int mapY = (int)y;

        // Si se sale del mapa o choca con pared > 0
        if (mapX < 0 || mapX >= mapW || mapY < 0 || mapY >= mapH || worldMap[mapY][mapX] > 0) {
            active = false; // Destruir proyectil
            // Aqu� podr�as reproducir sonido de impacto
        }
    }
};

