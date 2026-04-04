#pragma once
#include <SDL.h>
#include <iostream>
#include <vector>  

// Renombrado a EntityState o SpriteState para que sea universal
enum EntityState {
    STATE_IDLE,      // Parado / Estado por defecto (una antorcha encendida)
    STATE_WALKING,   // Opcional: para cuando el enemigo te persigue
    STATE_ATTACKING, // El enemigo ataca (los props normales nunca usarán esto)
    STATE_DYING,     // Reproduciendo animación de muerte (enemigo cayendo, o barril explotando)
    STATE_DEAD       // Ya murió (se queda en el suelo o desaparece)
};

class Sprite {
public:
    float x, y; // Posición en el mundo
    bool isDead = false;
    int hp;
    float scale;
    float zOffset;
    
    // SISTEMA DE ANIMACIÓN
    EntityState state;          // Estado actual
	SDL_Surface* defaultSurf;
    SDL_Surface* currentSurf;


    std::vector<SDL_Surface*> animIdle; 
    std::vector<SDL_Surface*> animDeath; // Lista de imágenes de muerte
    
    int animFrame;       // En qué cuadro vamos (0, 1, 2...)
    float animTimer;     // Tiempo acumulado para cambiar cuadro
    float animSpeed;     // Qué tan rápido cambia 

    Sprite(float px, float py, SDL_Surface* initialTexture, float sc);

    virtual ~Sprite();

    virtual void update(float deltaTime);

    void draw(Uint32* screenBuffer,
        const std::vector<float>& zBuffer,
        int SCREEN_WIDTH, int SCREEN_HEIGHT,
        float playerX, float playerY,
        float playerAngle, float FOV
        );

    virtual void takeDamage(int amount);

    void addDeathFrame(SDL_Surface* surf);
    void addIdleFrame(SDL_Surface* surf);
    virtual bool isPickup() { return false; }
};