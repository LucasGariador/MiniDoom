#pragma once
#include <SDL.h>
static class Utils
{
public:
	static SDL_Surface* LoadTexture(const char* path);
	static SDL_Texture* LoadTextSDL(const char* path, SDL_Renderer* renderer);
};

