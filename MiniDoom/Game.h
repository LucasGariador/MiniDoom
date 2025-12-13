#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <iostream>
#include "Sprite.h"
#include "Weapon.h"
#include "Projectile.h"
#include "Utils.h"

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
	float deltaTime;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // --- BUFFER DE VIDEO (Optimización) ---
    SDL_Texture* screenTexture;
    Uint32* screenBuffer;

    int screenWidth;
    int screenHeight;

	int floorW, floorH, ceilW, ceilH;

    // --- JUGADOR ---
    float playerX, playerY;
    float playerAngle;
    float FOV;
	float mouseSensitivity = 0.003f; // Ajusta este número si gira muy rápido o lento
	const float moveSpeed = 4.0f; // velocidad de movimiento
	bool playerIsMoving;
    const float playerRadius = 0.20f;  // “grosor” del jugador
    const float skin = 0.001f;  // evita quedar pegado al borde
    const float depth = 16.0f;

	// --- UI ---
	int health = 100;
	int ammo = 30;
	SDL_Rect healthBarBack = { 300, 550, 200, 25 }; // Fondo rojo oscuro
	SDL_Rect healthBarFront = { 300, 550, 200, 25 }; // Vida actual roja viva
	bool showUI = true;
    int crossW;
    int crossH;

    // --- MAPA ---
    const int MINI_SCALE = 6;              // píxeles por celda del mapa
    const int MINI_PADDING = 8;            // margen desde la esquina
    const int MINI_RAYS_STEP = 2;          // cada cuántas columnas dibujar un rayo (sube perf)
    bool showMinimap = true;               // toggle con tecla
    // (Idealmente esto iría en su propia clase Map, pero paso a paso)
    static const int mapWidth = 30;
    static const int mapHeight = 30;

    int worldMap[mapHeight][mapWidth] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,9,0,0,0,0,1,1,1,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,1,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,9,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,9,0,0,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1},
    { 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    };

    // --- RECURSOS ---
    SDL_Surface* wallSurface;
    Uint32* wallPixels;
    SDL_Surface* floorSurface;
    Uint32* floorPixels;
    Uint32* ceilPixels;
    TTF_Font* font = nullptr;
    SDL_Surface* crosshairSurf;
    SDL_Texture* crosshairTexture;
    // ... agrega aquí techo, arma, etc ...
    SDL_Surface* fireballTex;
    SDL_Surface* enemySurface;

    // --- ENTIDADES ---
    std::vector<Projectile*> projectiles;
    std::vector<Sprite*> sprites;
    Weapon playerStaff;
    std::vector<float> zBuffer;

    // --- HELPERS INTERNOS ---
    void renderWorld();   // Aquí metemos el bucle gigante de Raycasting
    void renderSprites(); // Aquí dibujamos enemigos y proyectiles
    void renderUI();      // Arma, mira, HUD
	void loadLevel();    // Carga sprites
    void MoveWithCollision(float& playerX, float& playerY,
        float dx, float dy,
        const int worldMap[mapHeight][mapWidth],
        float playerRadius, float skin);
    void DrawMinimap(SDL_Renderer* renderer,
        int mapWidth, int mapHeight, int worldMap[][30], // ajusta segundo dim si tu mapa cambia
        float playerX, float playerY,
        float playerAngle,
        float fov,             // tu FOV en radianes
        int screenWidth, int screenHeight,
        const std::vector<float>& zBuffer);
    void checkShooting(Sprite* enemy,
        float playerX, float playerY,
        float playerAngle,
        const std::vector<float>& zBuffer,
        int screenWidth);
};