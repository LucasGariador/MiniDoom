#include "Weapon.h"

// Carga los recursos UNA SOLA VEZ
void Weapon::init(SDL_Renderer* renderer, const char* pathIdle, const char* pathFire, bool weaponBob, float scale, int offset) {
    wScale = scale;
    hasWeponBob = weaponBob;
    wOffset = offset;
    // Cargar imágenes temporales
    SDL_Surface* surfIdle = Utils::LoadTexture(pathIdle);
    SDL_Surface* surfFire = Utils::LoadTexture(pathFire);

    if (!surfIdle || !surfFire) {
        std::cout << "Error cargando imagenes del arma: " << SDL_GetError() << std::endl;
        return;
    }

    // Convertir a Textura (GPU) y liberar la Surface (CPU)
    textureIdle = SDL_CreateTextureFromSurface(renderer, surfIdle);
    textureFire = SDL_CreateTextureFromSurface(renderer, surfFire);

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
    // --- CÁLCULO DE DIMENSIONES ---

    // el tamaño real de la textura
    int texW, texH;
    SDL_QueryTexture(currentTex, NULL, NULL, &texW, &texH);


    float scale = wScale;
    int drawH = (int)(SCREEN_HEIGHT * scale);

    // Mantener el aspecto
    float aspect = (float)texW / (float)texH;
    int drawW = (int)(drawH * aspect);

    // --- EFECTO DE MOVIMIENTO (Weapon Bob) ---
    int bobOffsetX = 0;
    int bobOffsetY = 0;

    if (isMoving && !isFiring && hasWeponBob) {
        bobTimer += deltaTime * 0.1f; // Velocidad del paso
        // Fórmula simple de oscilación
        bobOffsetX = (int)(cos(bobTimer) * 10); // Se mueve izquierda/derecha
        bobOffsetY = (int)(abs(sin(bobTimer)) * 10); // Se mueve arriba/abajo
    }
    else {
        // Resetear si se detiene
        bobTimer = 0;
    }

    // --- POSICIONAMIENTO FINAL ---

    int xOffset = wOffset; // Ajuste manual hacia la derecha

    // Retroceso visual
    int recoilY = isFiring ? 30 : 0;

    SDL_Rect destRect;
    // X: Pegado a la derecha - AnchoArma + Ajuste + Balanceo
    destRect.x = (SCREEN_WIDTH - drawW) + xOffset + bobOffsetX;

    // Y: Abajo de la pantalla - AlturaArma + Retroceso + Balanceo
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
		//Para sonido u otro efecto, agregar aquí
    }
}

// Limpieza de memoria al cerrar el juego
void Weapon::clean() {
    SDL_DestroyTexture(textureIdle);
    SDL_DestroyTexture(textureFire);
}