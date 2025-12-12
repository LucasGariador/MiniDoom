#pragma once
#include <SDL.h>
#include <iostream>
#include <vector>  

class Sprite {
public:
	float x, y; // Posición en el mundo
	bool isDead = false;
	int hp;
	SDL_Surface* surface; // Superficie de la imagen del sprite

	Sprite(float px, float py, SDL_Surface* surf);

	void draw(SDL_Renderer* renderer,
		const std::vector<float>& zBuffer,
		int SCREEN_WIDTH, int SCREEN_HEIGHT,
		float playerX, float playerY,
		float playerAngle, float FOV
		);

	void takeDamage(int amount) {
		if (isDead) return;
		hp -= amount;
		if (hp <= 0) {
			hp = 0;
			isDead = true;
			// Opcional: Cambiar la textura a una de "cadáver" aquí
			std::cout << "¡Enemigo eliminado!" << std::endl;
		}
	}
};
