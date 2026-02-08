#include <SDL.h>
#pragma once

struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
    // Puedes añadir un color para el efecto 'hover'
    SDL_Color color;
};
