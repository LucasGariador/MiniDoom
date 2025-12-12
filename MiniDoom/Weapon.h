#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>

struct Weapon {
    // Usamos SDL_Texture para dibujar, es mucho más rápido que Surface
    SDL_Texture* textureIdle = nullptr;
    SDL_Texture* textureFire = nullptr;

    bool isFiring = false;
    float fireTimer = 0.0f;
    const float FIRE_DURATION = 0.2f;

    // Variables para el efecto "Weapon Bob" (movimiento al caminar)
    float bobTimer = 0.0f;

    // Carga los recursos UNA SOLA VEZ
    void init(SDL_Renderer* renderer, const char* pathIdle, const char* pathFire) {
        // Cargar imágenes temporales
        SDL_Surface* surfIdle = IMG_Load(pathIdle); // O SDL_LoadBMP si es bmp
        SDL_Surface* surfFire = IMG_Load(pathFire);

        if (!surfIdle || !surfFire) {
            std::cout << "Error cargando imagenes del arma: " << SDL_GetError() << std::endl;
            return;
        }

        // Convertir a Textura (GPU) y liberar la Surface (CPU)
        textureIdle = SDL_CreateTextureFromSurface(renderer, surfIdle);
        textureFire = SDL_CreateTextureFromSurface(renderer, surfFire);

        SDL_FreeSurface(surfIdle);
        SDL_FreeSurface(surfFire);
    }

    // Limpieza de memoria al cerrar el juego
    void clean() {
        SDL_DestroyTexture(textureIdle);
        SDL_DestroyTexture(textureFire);
    }

    void draw(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, float deltaTime, bool isMoving) {
        SDL_Texture* currentTex = textureIdle;

        // Lógica de disparo
        if (isFiring) {
            fireTimer -= deltaTime;
            if (fireTimer <= 0) isFiring = false;
            else currentTex = textureFire;
        }

        if (!currentTex) return;

        // --- CÁLCULO DE DIMENSIONES ---

        // Obtenemos el tamaño real de la textura
        int texW, texH;
        SDL_QueryTexture(currentTex, NULL, NULL, &texW, &texH);

        // Escala: 1.2 veces la altura de la pantalla (Imponente)
        float scale = 1.2f;
        int drawH = (int)(SCREEN_HEIGHT * scale);

        // Mantenemos el aspecto
        float aspect = (float)texW / (float)texH;
        int drawW = (int)(drawH * aspect);

        // --- EFECTO DE MOVIMIENTO (Weapon Bob) ---
        int bobOffsetX = 0;
        int bobOffsetY = 0;

        if (isMoving && !isFiring) {
            bobTimer += deltaTime * 10.0f; // Velocidad del paso
            // Fórmula simple de oscilación
            bobOffsetX = (int)(cos(bobTimer) * 10); // Se mueve izquierda/derecha
            bobOffsetY = (int)(abs(sin(bobTimer)) * 10); // Se mueve arriba/abajo (semicírculo)
        }
        else {
            // Resetear suavemente si se detiene (opcional, aquí reseteo directo)
            bobTimer = 0;
        }

        // --- POSICIONAMIENTO FINAL ---

        int xOffset = 50; // Ajuste manual hacia la derecha

        // Retroceso visual
        int recoilY = isFiring ? 30 : 0;

        SDL_Rect destRect;
        // X: Pegado a la derecha - AnchoArma + Ajuste + Balanceo
        destRect.x = (SCREEN_WIDTH - drawW) + xOffset + bobOffsetX;

        // Y: Abajo de la pantalla - AlturaArma + Retroceso + Balanceo
        destRect.y = (SCREEN_HEIGHT - drawH) + recoilY + bobOffsetY;

        destRect.w = drawW;
        destRect.h = drawH;

        SDL_RenderCopy(renderer, currentTex, NULL, &destRect);
    }

    void shoot() {
        if (!isFiring) {
            isFiring = true;
            fireTimer = FIRE_DURATION;
            // Reproducir sonido aquí
        }
    }
};