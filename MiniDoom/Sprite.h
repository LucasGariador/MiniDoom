#pragma once
#include <SDL.h>
#include <vector>   

class Sprite {
public:
	float x, y; // Posición en el mundo
	int hp;
	SDL_Surface* surface; // Superficie de la imagen del sprite

	Sprite(float px, float py, SDL_Surface* surf);

	void draw(SDL_Renderer* renderer,
		const std::vector<float>& zBuffer,
		int SCREEN_WIDTH, int SCREEN_HEIGHT,
		float playerX, float playerY,
		float playerAngle, float FOV
		);
};
