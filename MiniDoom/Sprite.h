#pragma once
#include <SDL.h>
#include <iostream>
#include <vector>  

enum EnemyState {
	STATE_IDLE,    // Parado / Caminando
	STATE_ATTACKING, // Atacando
	STATE_DYING,   // Reproduciendo animación de muerte
	STATE_DEAD     // Ya murió (se queda en el suelo o desaparece)
};

class Sprite {
public:
	float x, y; // Posición en el mundo
	bool isDead = false;
	int hp;
	float scale;
	float zOffset;
	// SISTEMA DE ANIMACIÓN
	EnemyState state;           // Estado actual
	SDL_Surface* currentSurf;   // La imagen que se dibuja ACTUALMENTE (puntero a una de la lista)

	// Animaciones
	std::vector<SDL_Surface*> animDeath; // Lista de imágenes de muerte
	int animFrame;       // En qué cuadro vamos (0, 1, 2...)
	float animTimer;     // Tiempo acumulado para cambiar cuadro
	float animSpeed;     // Qué tan rápido cambia 

	Sprite(float px, float py, SDL_Surface* initialTexture, float sc);

	virtual ~Sprite();

	void virtual update(float deltaTime);

	void draw(Uint32* screenBuffer,
		const std::vector<float>& zBuffer,
		int SCREEN_WIDTH, int SCREEN_HEIGHT,
		float playerX, float playerY,
		float playerAngle, float FOV
		);

	void takeDamage(int amount);

	void addDeathFrame(SDL_Surface* surf);

	virtual bool isPickup() { return false; }
};
