#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include "Sprite.h"
#include "Weapon.h"
#include "Projectile.h"

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int width, int height);

    void handleEvents(); // Teclado y Mouse
    void update();       // Movimiento, Colisiones, IA
    void render();       // Raycasting, Dibujado
    void clean();        // Destruir ventanas y texturas

    bool running() { return isRunning; }

private:
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // --- BUFFER DE VIDEO (Optimización) ---
    SDL_Texture* screenTexture;
    Uint32* screenBuffer;
    int screenWidth;
    int screenHeight;

    // --- JUGADOR ---
    float playerX, playerY;
    float playerAngle;
    float FOV;

    // --- MAPA ---
    // (Idealmente esto iría en su propia clase Map, pero paso a paso)
    static const int mapWidth = 30;
    static const int mapHeight = 30;

    int worldMap[mapHeight][mapWidth] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // --- RECURSOS ---
    SDL_Surface* wallSurface;
    Uint32* wallPixels;
    SDL_Surface* floorSurface;
    Uint32* floorPixels;
    // ... agrega aquí techo, arma, etc ...

    // --- ENTIDADES ---
    Sprite* enemy;
    Weapon shotgun;
    std::vector<Projectile*> projectiles;
    std::vector<float> zBuffer;

    // --- HELPERS INTERNOS ---
    void renderWorld();   // Aquí metemos el bucle gigante de Raycasting
    void renderSprites(); // Aquí dibujamos enemigos y proyectiles
    void renderUI();      // Arma, mira, HUD
};