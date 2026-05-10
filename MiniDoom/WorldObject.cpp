#include "WorldObject.hpp"
#include <iostream>

// --- CONSTRUCTOR ---
WorldObject::WorldObject(float px, float py, SDL_Surface* tex, float sc, bool solid, bool interactable, bool destructible)
    : Sprite(px, py, tex, sc), isSolid(solid), isInteractable(interactable), isDestructible(destructible)
{
    state = STATE_IDLE; 
    isPortal = false;
}

WorldObject::~WorldObject()
{
}

// --- ACTUALIZACIÓN DE LÓGICA Y ANIMACIÓN ---
void WorldObject::update(float deltaTime) {
    
    Sprite::update(deltaTime);


    if (state == STATE_IDLE && !animIdle.empty()) {
        animTimer += deltaTime;

        if (animTimer >= animSpeed) {
            animTimer = 0.0f;
            animFrame++;


            if (animFrame >= animIdle.size()) {
                animFrame = 0; 
            }
            
            currentSurf = animIdle[animFrame];
        }
    }
}

// --- SISTEMA DE DAÑO ---
void WorldObject::takeDamage(int amount) {
    if (!isDestructible) {
        return; 
    }

    Sprite::takeDamage(amount);
}

// --- SISTEMA DE INTERACCIÓN ---
void WorldObject::onInteract() {
    if (!isInteractable) {
        return; 
    }

    std::cout << "[MUNDO] Interaccion activada en el objeto (X: " << x << ", Y: " << y << ")" << std::endl;
    
    // Aquí en el futuro puedes hacer un switch/if para determinar qué hace el objeto:
    // - Si es un interruptor: Cambiar la textura a "encendido" y abrir una puerta.
    // - Si es un terminal: Mostrar un mensaje en la UI.
    // - Si es el portal: Cambiar 'Game::currentState' o llamar a 'loadNextLevel()'.
}

// --- CARGA DE FRAMES ---
void WorldObject::addIdleFrame(SDL_Surface* surf) {
    animIdle.push_back(surf);
}