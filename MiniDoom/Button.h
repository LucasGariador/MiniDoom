#pragma once

#include <SDL.h>
struct Button {
    SDL_Rect rect;
    SDL_Texture* texture;
    // hover
    SDL_Color color;
};
