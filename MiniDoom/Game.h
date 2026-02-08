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
#include "ResourceManager.h"
#include "Pickup.h"
#include "Enemy.h"
#include "Button.h"

class Game {
public:
    Game();
    ~Game();

    enum GameState { MAINMENU, PLAYING, EXIT };
    GameState currentState = MAINMENU;

    bool init(const char* title, int width, int height);

    void handleEvents();
    void update();
    void render();
    void clean();

    bool running() { return isRunning; }

private:
    bool isRunning;
	float deltaTime;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // --- BUFFER DE VIDEO ---
    SDL_Texture* screenTexture;
    Uint32* screenBuffer;

    int screenWidth;
    int screenHeight;

	int floorW, floorH, ceilW, ceilH;

    // --- JUGADOR ---
    float playerX, playerY;
    float playerAngle;
    float FOV;
	float mouseSensitivity = 0.003f;
	const float moveSpeed = 4.0f;
	bool playerIsMoving;
    const float playerRadius = 0.20f;
    const float skin = 0.001f;
    const float depth = 16.0f;
   

	// --- UI ---
    Button playButton;

	int health = 100;
	int ammo = 30;
	SDL_Rect healthBarBack = { 300, 550, 200, 25 }; // Red
	SDL_Rect healthBarFront = { 300, 550, 200, 25 }; // Light Red
	SDL_Rect textRect;
	bool showUI = true;
    int crossW;
    int crossH;
	float damageFlashTimer = 0.0f;

    // --- MAPA ---
    const int MINI_SCALE = 6;
    const int MINI_PADDING = 8;
    const int MINI_RAYS_STEP = 2;
    bool showMinimap = true;

    static const int mapWidth = 30;
    static const int mapHeight = 30;

    int worldMap[mapHeight][mapWidth] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,7,0,0,0,0,1,0,0,0,0,0,0,0,0,1,5,0,0,0,0,0,0,0,0,0,0,0,4,1},
    {1,0,0,0,0,0,2,0,0,9,0,0,0,0,0,2,0,0,0,0,0,9,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,3,0,0,0,0,0,0,0,0,3,3,2,1,0,1,2,3,3,0,0,0,0,0,1},
    {1,1,2,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,9,0,0,0,0,1,2,3,0,3,2,1,0,0,0,9,0,0,0,0,0,0,9,0,0,0,1},
    {1,0,0,0,0,0,0,0,2,4,0,0,0,5,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,3,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,2,0,2,1,1,1,1,0,0,0,0,0,1,1,1,2,3,0,3,2,1,1,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,9,0,0,0,0,0,0,1,2,0,2,1,0,0,0,0,0,0,0,9,0,0,0,0,0,9,0,1},
    {1,0,0,0,0,0,0,0,0,2,5,0,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,9,0,0,0,0,3,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,0,1,1,1,2,2,2,0,0,0,2,2,2,1,1,1,0,1,1,1,1,2,2,2,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,4,0,0,0,3,3,3,0,0,0,0,0,3,3,3,0,0,0,9,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,3,5,0,0,0,0,0,0,0,4,3,0,0,0,0,0,0,0,0,0,0,9,0,1},
    {1,1,2,3,2,1,3,0,0,9,0,9,0,0,0,0,3,1,2,3,2,1,1,0,1,1,2,2,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,9,0,0,0,1},
    {1,0,0,0,0,1,4,0,0,0,9,0,0,0,0,0,5,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,5,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,4,0,0,0,5,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    };

    // --- RECURSOS ---
    SDL_Surface* wallSurface;
	SDL_Surface* wallSurface_2;
    SDL_Surface* wallSurface_3;
    Uint32* textures[3];
    Uint32* wallPixels;
    SDL_Surface* floorSurface;
    Uint32* floorPixels;
    Uint32* ceilPixels;
    TTF_Font* font = nullptr;
    SDL_Surface* crosshairSurf;
    SDL_Texture* crosshairTexture;
    SDL_Texture* textTexture;

    SDL_Surface* fireballTex;
	SDL_Surface* handsSurf;
	SDL_Texture* backgroundTexture;


    // --- ENTIDADES ---
    std::vector<Projectile*> projectiles;
    std::vector<Sprite*> sprites;
    std::vector<SDL_Surface*> textureEnemyDie;
    Weapon playerStaff;
	Weapon handsWeapon;
	Weapon* currentWeapon;
    SDL_Surface* textureEnemyIdle;
    std::vector<float> zBuffer;
    

    // --- HELPERS INTERNOS ---
    void renderWorld();
    void renderSprites();
    void renderUI();
	void loadLevel();
    void MoveWithCollision(float& playerX, float& playerY,
        float dx, float dy,
        const int worldMap[mapHeight][mapWidth],
        float playerRadius, float skin);
    void drawMinimap(SDL_Renderer* renderer,
        int mapWidth, int mapHeight, int worldMap[][30],
        float playerX, float playerY,
        float playerAngle,
        float fov,
        int screenWidth, int screenHeight,
        const std::vector<float>& zBuffer);
    void checkShooting(Sprite* enemy,
        float playerX, float playerY,
        float playerAngle,
        const std::vector<float>& zBuffer,
        int screenWidth);
	template <typename T>
    void cleanDeadEntities(std::vector<T*>& list);
};