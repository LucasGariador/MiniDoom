#pragma once
#include "Sprite.h"

class WorldObject : public Sprite {
public:

    bool isSolid;
    bool isInteractable;
    bool isDestructible;
    bool isPortal;

    WorldObject(float px, float py, SDL_Surface* tex, float sc, bool solid, bool interactable, bool destructible);
    ~WorldObject();


    void addIdleFrame(SDL_Surface* surf);


    virtual void update(float deltaTime) override;


    virtual void onInteract(); 


    virtual void takeDamage(int amount) override; 

private:

    std::vector<SDL_Surface*> animIdle;
};