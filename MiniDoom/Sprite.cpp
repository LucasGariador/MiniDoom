#include "Sprite.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

Sprite::Sprite(float px, float py, SDL_Surface* initialTexture, float sc)
	: x(px), y(py), currentSurf(initialTexture), hp(100), scale(sc), zOffset(0.0f)
{
    state = STATE_IDLE;
    animFrame = 0;
    animTimer = 0.0f;
    animSpeed = 0.15f; // Cambia de imagen cada 0.15 segundos
}

Sprite::~Sprite()
{
}

void Sprite::draw(Uint32* screenBuffer, // <--- CAMBIO 1: Escribimos en memoria, no en renderer
    const std::vector<float>& zBuffer,
    int SCREEN_WIDTH, int SCREEN_HEIGHT,
    float playerX, float playerY,
    float playerAngle, float FOV)
{
    if (!currentSurf) return;

    // Safety check: Ensure surface is 32-bit (4 bytes per pixel)

    // If your BMP is 24-bit, this code will crash without conversion.

    if (currentSurf->format->BytesPerPixel != 4) {

        std::cerr << "Error: Sprite surface must be 32-bit (ARGB/RGBA).\n";

        return;

    }

    const int textureWidth = currentSurf->w;
    const int textureHeight = currentSurf->h;

    // 1. Posición Relativa
    const float dx = x - playerX;
    const float dy = y - playerY;

    // 2. Transformación con Matriz
    // Esto es más estable que atan2 para sprites, pero tu método atan2 también funciona.
    // Usaremos tu lógica de atan2 para no confundirte, pero corregida para la escala.

    const float distance = sqrtf(dx * dx + dy * dy);
    if (distance < 0.1f) return;

    float spriteAngle = atan2f(dy, dx) - playerAngle;
    while (spriteAngle < -M_PI) spriteAngle += 2.0f * M_PI;
    while (spriteAngle > M_PI) spriteAngle -= 2.0f * M_PI;

    const float perpDist = distance * cosf(spriteAngle);
    if (perpDist <= 0.1f) return;

    // 3. Calcular Dimensiones en Pantalla
    const float halfW = SCREEN_WIDTH * 0.5f;
    const float halfH = SCREEN_HEIGHT * 0.5f;

    const float projX = halfW * (1.0f + tanf(spriteAngle) / tanf(FOV * 0.5f));

    // --- AQUI APLICAMOS LA ESCALA (CAMBIO 2) ---
    // Multiplicamos por 'this->scale'
    const int spriteScreenH = abs((int)(SCREEN_HEIGHT / perpDist)) * scale;
    const int spriteScreenW = abs((int)((float)spriteScreenH * (float)textureWidth / (float)textureHeight));

    // 4. Calcular límites de dibujo
    int drawStartX = (int)(projX - spriteScreenW / 2);
    int drawEndX = (int)(projX + spriteScreenW / 2);


    // Si quieres que los enemigos pisen el suelo (no floten al escalar):
    int vMoveScreen = (int)(zOffset / perpDist); // Puedes ajustar este 0.0f

    int drawStartY = (int)(halfH - spriteScreenH / 2) + vMoveScreen; // <--- SUMAR AQUÍ
    int drawEndY = (int)(halfH + spriteScreenH / 2) + vMoveScreen;   // <--- SUMAR AQUÍ
    // drawStartY += vMoveScreen;
    // drawEndY += vMoveScreen;

    if (drawStartX >= SCREEN_WIDTH || drawEndX < 0 || drawStartY >= SCREEN_HEIGHT || drawEndY < 0) return;

    // 5. Setup de Textura
    int loopStartX = drawStartX;
    if (loopStartX < 0) loopStartX = 0;
    int loopEndX = drawEndX;
    if (loopEndX >= SCREEN_WIDTH) loopEndX = SCREEN_WIDTH - 1;

    int loopStartY = drawStartY; // Renombrado para claridad
    if (loopStartY < 0) loopStartY = 0;
    int loopEndY = drawEndY;
    if (loopEndY >= SCREEN_HEIGHT) loopEndY = SCREEN_HEIGHT - 1;

    // Punteros directos
    Uint32* texPixels = (Uint32*)currentSurf->pixels;

    // Pasos de textura
    float stepX = (float)textureWidth / (float)spriteScreenW;
    float stepY = (float)textureHeight / (float)spriteScreenH;

    float texPosX = (loopStartX - (projX - spriteScreenW / 2)) * stepX;
    // 6. Bucle de Renderizado (OPTIMIZADO)
    for (int x = loopStartX; x <= loopEndX; ++x) {

        // Z-Buffer Check: ¿Está el sprite delante de la pared?
        if (perpDist < zBuffer[x]) {

            int texX = (int)texPosX;
            if (texX >= textureWidth) texX = textureWidth - 1; // Clamp simple

            float texPosY = (loopStartY - drawStartY) * stepY;

            for (int y = loopStartY; y <= loopEndY; ++y) {
                int texY = (int)texPosY;
                if (texY >= textureHeight) texY = textureHeight - 1; // Clamp simple

                int index = texY * textureWidth + texX;

                int maxSize = (currentSurf->pitch / 4) * currentSurf->h;
                // Obtener color (Bitwise AND es más rápido que modulo, pero requiere potencias de 2. Usamos acceso directo)
                if (index >= 0 && index < maxSize) {
                    Uint32 color = texPixels[texY * textureWidth + texX];

                    // Transparencia: Chequear Canal Alpha (Byte más alto)
                    // Si el color es 0x00RRGGBB, es transparente.
                    if ((color >> 24) != 0) {
                        // CAMBIO 3: Escribir directo al buffer
                        // Esto es 100 veces más rápido que SDL_RenderDrawPoint
                        screenBuffer[y * SCREEN_WIDTH + x] = color;
                    }
                }

                texPosY += stepY;
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