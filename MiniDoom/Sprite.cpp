#include "Sprite.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

Sprite::Sprite(float px, float py, SDL_Surface* initialTexture)
    : x(px), y(py), currentSurf(initialTexture), hp(100)
{
    state = STATE_IDLE;
    animFrame = 0;
    animTimer = 0.0f;
    animSpeed = 0.15f; // Cambia de imagen cada 0.15 segundos
}

Sprite::~Sprite()
{
    // Liberar superficies de animación
    for (SDL_Surface* surf : animDeath) {
        SDL_FreeSurface(surf);
    }
    // No liberamos currentSurf porque apunta a una de las de animDeath
}

void Sprite::draw(SDL_Renderer* renderer,
    const std::vector<float>& zBuffer,
    int SCREEN_WIDTH, int SCREEN_HEIGHT,
    float playerX, float playerY,
    float playerAngle, float FOV)
{
    if (!currentSurf) return;

    if (currentSurf->format->BytesPerPixel != 4) {
        std::cout << "ERROR FATAL: La imagen no es de 32 bits. Es de: "
            << (int)currentSurf->format->BytesPerPixel << " bytes." << std::endl;
        return; // Detener dibujo para no crashear
    }

    // Safety check: Ensure surface is 32-bit (4 bytes per pixel)
    // If your BMP is 24-bit, this code will crash without conversion.
    if (currentSurf->format->BytesPerPixel != 4) {
        std::cerr << "Error: Sprite surface must be 32-bit (ARGB/RGBA).\n";
        return;
    }

    const int textureWidth = currentSurf->w;
    const int textureHeight = currentSurf->h;

    // 1. Relative position
    const float dx = x - playerX;
    const float dy = y - playerY;

    // Opt: Use distance squared for the quick check to avoid sqrtf if not needed yet
    // But we need actual distance for projection, so we keep sqrtf.
    const float distance = sqrtf(dx * dx + dy * dy);

    if (distance <= 0.1f) return;    // Clipping near
    if (distance > 20.0f) return;    // Clipping far

    // 2. Angle Calculation
    float spriteAngle = atan2f(dy, dx) - playerAngle;

    // Normalize angle to [-PI, PI]
    while (spriteAngle < -M_PI) spriteAngle += 2.0f * M_PI;
    while (spriteAngle > M_PI) spriteAngle -= 2.0f * M_PI;

    // FIX 1: REMOVED "Center FOV" check. 
    // We handle clipping by checking if the calculated X coordinates are on screen later.
    // If we return here, sprites "pop" when looking at their edges.

    // 3. Screen Projection
    // Correct fisheye distortion for distance
    const float perpDist = distance * cosf(spriteAngle);
    if (perpDist <= 0.1f) return;

    const float halfW = SCREEN_WIDTH * 0.5f;
    const float halfH = SCREEN_HEIGHT * 0.5f;

    // Calculate Screen X
    // Note: This tan() math works, but Transform Matrix method is often more stable.
    // We will stick to your method for consistency.
    const float projX = halfW * (1.0f + tanf(spriteAngle) / tanf(FOV * 0.5f));

    // Calculate Dimensions
    const int spriteScreenH = abs((int)(SCREEN_HEIGHT / perpDist)); // abs to prevent negative
    const int spriteScreenW = abs((int)((float)spriteScreenH * (float)textureWidth / (float)textureHeight));

    // 4. Calculate Drawing Bounds
    int drawStartX = (int)(projX - spriteScreenW / 2);
    int drawEndX = (int)(projX + spriteScreenW / 2);
    int drawStartY = (int)(halfH - spriteScreenH / 2);
    int drawEndY = (int)(halfH + spriteScreenH / 2);

    // FIX 2: Check if the *entire* sprite is off-screen
    if (drawStartX >= SCREEN_WIDTH || drawEndX < 0 ||
        drawStartY >= SCREEN_HEIGHT || drawEndY < 0) {
        return;
    }

    // 5. Texture Coordinate Setup (Optimization)
    // We calculate the start/end in screen space, but we need to know 
    // where to start reading the texture if the sprite is clipped by the screen edge.

    int loopStartX = drawStartX;
    int loopEndX = drawEndX;

    // Clamp loops to screen, but calculate texture offset
    if (loopStartX < 0) loopStartX = 0;
    if (loopEndX >= SCREEN_WIDTH) loopEndX = SCREEN_WIDTH - 1;

    if (drawStartY < 0) drawStartY = 0;
    if (drawEndY >= SCREEN_HEIGHT) drawEndY = SCREEN_HEIGHT - 1;

    // Direct pixel access
    Uint32* pixels32 = (Uint32*)currentSurf->pixels;
    int pitch32 = currentSurf->pitch / 4;

    // Pre-calculate steps to avoid division in the loop
    float stepX = (float)textureWidth / (float)spriteScreenW;
    float stepY = (float)textureHeight / (float)spriteScreenH;

    // Starting texture X offset based on clipping
    float texPosX = (loopStartX - (projX - spriteScreenW / 2)) * stepX;

    // 6. Render Loop
    for (int x = loopStartX; x <= loopEndX; ++x) {

        // Z-Buffer Check (Wall Occlusion)
        if (perpDist < zBuffer[x]) {

            int texX = (int)texPosX;
            // Safety clamp for texture index
            if (texX >= textureWidth) texX = textureWidth - 1;

            // Calculate starting Y texture coordinate based on clipping
            float texPosY = (drawStartY - (halfH - spriteScreenH / 2)) * stepY;

            for (int y = drawStartY; y <= drawEndY; ++y) {

                int texY = (int)texPosY;
                if (texY >= textureHeight) texY = textureHeight - 1; // Safety

                // 1. Asegurar texX
                if (texX < 0) texX = 0;
                if (texX >= textureWidth) texX = textureWidth - 1;

                // 2. Asegurar texY
                if (texY < 0) texY = 0;
                if (texY >= textureHeight) texY = textureHeight - 1;

                // 3. CALCULO SEGURO
                int index = texY * pitch32 + texX;
                int maxIndex = (currentSurf->pitch / 4) * currentSurf->h; // Tamaño total del array en Uint32

                // 4. DOBLE VERIFICACIÓN (El "salvavidas")
                if (index >= 0 && index < maxIndex) {
                    Uint32 color = pixels32[index]; // <--- AQUI YA NO DEBERIA FALLAR

                    // Fast alpha check (assuming ARGB or ABGR format)
                    // If the top 8 bits are 0 OR the color is specific magenta/white
                    if ((color >> 24) != 0) {
                        // Manual color key check (if not using Alpha channel)
                        // Example: Filter out Pure White (0xFFFFFFFF) or Magenta (0xFFFF00FF)
                        if (color != 0xFFFFFFFF) {

                            Uint8 r, g, b, a;
                            SDL_GetRGBA(color, currentSurf->format, &r, &g, &b, &a);

                            // Performance Warning: 
                            // SDL_RenderDrawPoint is SLOW. 
                            // Ideally, you should write to a Uint32 texture buffer instead.
                            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                            SDL_RenderDrawPoint(renderer, x, y);
                        }
                    }
                    texPosY += stepY;
                }


            }
        }
        texPosX += stepX;
    }
}

void Sprite::update(float deltaTime) {
    if (state == STATE_DYING) {
        animTimer += deltaTime;

        // ¿Es hora de cambiar de frame?
        if (animTimer >= animSpeed) {
            animTimer = 0; // Reset timer
            animFrame++;   // Siguiente imagen

            // Verificar si terminamos la animación
            if (animFrame < animDeath.size()) {
                // Actualizamos la imagen visible
                currentSurf = animDeath[animFrame];
            }
            else {
                // Se acabaron los frames, el enemigo está oficialmente MUERTO
                state = STATE_DEAD;
                // Dejamos la última imagen (el cadáver en el suelo)
                currentSurf = animDeath.back();

                // Opcional: Hacerlo no colisionable (atravesable)
            }
        }
    }
}

void Sprite::takeDamage(int amount) {
    if (state == STATE_DEAD || state == STATE_DYING) return; // No rematar muertos

    hp -= amount;
    if (hp <= 0) {
        hp = 0;
        state = STATE_DYING; // <--- AQUÍ EMPIEZA LA ANIMACIÓN
        animFrame = 0;
        animTimer = 0;

        // Cambiar inmediatamente al primer frame de muerte si existe
        if (!animDeath.empty()) {
            currentSurf = animDeath[0];
        }
    }
}

void Sprite::addDeathFrame(SDL_Surface* surf) {
    animDeath.push_back(surf);
}