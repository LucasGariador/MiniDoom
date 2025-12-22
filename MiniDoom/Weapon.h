#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include "ResourceManager.h"

struct Weapon {
    
    SDL_Texture* textureIdle = nullptr;
    SDL_Texture* textureFire = nullptr;

    bool isFiring = false;
    float fireTimer = 0.0f;
    const float FIRE_DURATION = 0.2f;
	bool hasWeponBob = true;
	float wScale = 1.0f;
	int wOffset = 0.0f;

    // Variables para el efecto Weapon Bob
    float bobTimer = 0.0f;

    // Carga los recursos UNA SOLA VEZ
    void init(SDL_Renderer* renderer, const char* pathIdle, const char* pathFire, bool weaponBob, float scale, int offset);

    // Limpieza de memoria al cerrar el juego
    void clean();

    void draw(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, float deltaTime, bool isMoving);

    void shoot();
};