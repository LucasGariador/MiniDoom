#include "Weapon.h"

// Carga los recursos UNA SOLA VEZ
void Weapon::init(SDL_Renderer* renderer, const char* pathIdle, const char* pathFire, bool weaponBob, float scale, int offset) {
    wScale = scale;
    hasWeponBob = weaponBob;
    wOffset = offset;
    
    SDL_Surface* surfIdle = Utils::LoadTexture(pathIdle);
    SDL_Surface* surfFire = Utils::LoadTexture(pathFire);

    if (!surfIdle || !surfFire) {
        std::cout << "Error cargando imagenes del arma: " << SDL_GetError() << std::endl;
        return;
    }

    textureIdle = SDL_CreateTextureFromSurface(renderer, surfIdle);
    textureFire = SDL_CreateTextureFromSurface(renderer, surfFire);

    // --- NUEVO: Cachear las dimensiones aquí ---
    // Usamos la textura Idle como referencia (asumiendo que Idle y Fire miden lo mismo)
    SDL_QueryTexture(textureIdle, NULL, NULL, &texW, &texH);

    SDL_FreeSurface(surfIdle);
    SDL_FreeSurface(surfFire);
}

void Weapon::draw(SDL_Renderer* renderer, int SCREEN_WIDTH, int SCREEN_HEIGHT, float deltaTime, bool isMoving) {
    SDL_Texture* currentTex = textureIdle;

    // Lógica de disparo
    if (isFiring) {
        fireTimer -= deltaTime;
        if (fireTimer <= 0) isFiring = false;
        else currentTex = textureFire;
    }

    if (!currentTex) return;

    // --- CÁLCULO DE DIMENSIONES (Optimizado) ---
    int drawH = (int)(SCREEN_HEIGHT * wScale);
    float aspect = (float)texW / (float)texH; // Usamos las variables cacheadas
    int drawW = (int)(drawH * aspect);

    // --- EFECTO DE MOVIMIENTO (Weapon Bob) ---
    int bobOffsetX = 0;
    int bobOffsetY = 0;

    if (isMoving && !isFiring && hasWeponBob) {
        bobTimer += deltaTime * 0.1f; 
        bobOffsetX = (int)(cos(bobTimer) * 10); 
        bobOffsetY = (int)(abs(sin(bobTimer)) * 10); 
    }
    else {
        bobTimer = 0;
    }

    // --- POSICIONAMIENTO FINAL ---
    int xOffset = wOffset; 
    int recoilY = isFiring ? 30 : 0;

    SDL_Rect destRect;
    destRect.x = (SCREEN_WIDTH - drawW) + xOffset + bobOffsetX;
    destRect.y = (SCREEN_HEIGHT - drawH) + recoilY + bobOffsetY;
    destRect.w = drawW;
    destRect.h = drawH;

    SDL_RenderCopy(renderer, currentTex, NULL, &destRect);
}

 void Weapon::shoot() {

std::cout << "Firing weapon, time left: " << fireTimer << " seconds\n";

if (!isFiring) {

isFiring = true;

fireTimer = FIRE_DURATION;

//Para sonido u otro efecto, agregar aqu�

}

}


// Limpieza de memoria al cerrar el juego

void Weapon::clean() {

SDL_DestroyTexture(textureIdle);

SDL_DestroyTexture(textureFire);

} 