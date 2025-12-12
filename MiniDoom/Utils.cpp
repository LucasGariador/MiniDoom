#include "Utils.h"
#include <SDL.h>
#include <SDL_image.h> // <--- NUEVA LIBRERÍA
#include <iostream>

    SDL_Surface* Utils::LoadTexture(const char* path) {
        // 1. Cargar imagen usando IMG_Load (soporta PNG, JPG, BMP)
        SDL_Surface* tempSurface = IMG_Load(path);

        if (!tempSurface) {
            std::cerr << "Error cargando imagen " << path << ": " << IMG_GetError() << std::endl;
            return nullptr;
        }

        // 2. CONVERTIR A FORMATO 32-BITS (RGBA8888)
        // Esto es CRÍTICO para tu motor de Raycasting. 
        // Aunque el PNG ya traiga transparencia, necesitamos asegurar 
        // que el orden de los bytes sea el que tu bucle de dibujo espera.
        SDL_Surface* optimizedSurface = SDL_ConvertSurfaceFormat(tempSurface, SDL_PIXELFORMAT_RGBA8888, 0);

        // 3. Limpiar la superficie temporal
        SDL_FreeSurface(tempSurface);

        if (!optimizedSurface) {
            std::cerr << "Error convirtiendo superficie: " << SDL_GetError() << std::endl;
            return nullptr;
        }

        return optimizedSurface;
    }