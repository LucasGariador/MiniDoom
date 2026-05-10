#include "Sprite.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "Game.h"

Sprite::Sprite(float px, float py, SDL_Surface* initialTexture, float sc)
	: x(px), y(py), currentSurf(initialTexture), hp(100), scale(sc), zOffset(0.0f)
{
    state = STATE_IDLE;
    animFrame = 0;
    animTimer = 0.0f;
    animSpeed = 0.15f; // Cambia de imagen cada 0.15 segundos
    defaultSurf = currentSurf;
}

Sprite::~Sprite()
{
}

void Sprite::draw(Uint32* screenBuffer,
    const std::vector<float>& zBuffer,
    int SCREEN_WIDTH, int SCREEN_HEIGHT,
    float playerX, float playerY,
    float playerAngle, float FOV, bool debugMode)
{
    if (!currentSurf) return;

    if (currentSurf->format->BytesPerPixel != 4) {

        std::cerr << "Error: Sprite surface must be 32-bit (ARGB/RGBA).\n";

        return;

    }

    const int textureWidth = currentSurf->w;
    const int textureHeight = currentSurf->h;

    const float dx = x - playerX;
    const float dy = y - playerY;



    const float distance = sqrtf(dx * dx + dy * dy);
    if (distance < 0.1f) return;

    float spriteAngle = atan2f(dy, dx) - playerAngle;
    while (spriteAngle < -M_PI) spriteAngle += 2.0f * M_PI;
    while (spriteAngle > M_PI) spriteAngle -= 2.0f * M_PI;

    const float perpDist = distance * cosf(spriteAngle);
    if (perpDist <= 0.1f) return;


    const float halfW = SCREEN_WIDTH * 0.5f;
    const float halfH = SCREEN_HEIGHT * 0.5f;

    const float projX = halfW * (1.0f + tanf(spriteAngle) / tanf(FOV * 0.5f));

    const int spriteScreenH = abs((int)(SCREEN_HEIGHT / perpDist)) * scale;
    const int spriteScreenW = abs((int)((float)spriteScreenH * (float)textureWidth / (float)textureHeight));

    int drawStartX = (int)(projX - spriteScreenW / 2);
    int drawEndX = (int)(projX + spriteScreenW / 2);


    int vMoveScreen = (int)(zOffset / perpDist);

    int drawStartY = (int)(halfH - spriteScreenH / 2) + vMoveScreen;
    int drawEndY = (int)(halfH + spriteScreenH / 2) + vMoveScreen;


    if (drawStartX >= SCREEN_WIDTH || drawEndX < 0 || drawStartY >= SCREEN_HEIGHT || drawEndY < 0) return;


    int loopStartX = drawStartX;
    if (loopStartX < 0) loopStartX = 0;
    int loopEndX = drawEndX;
    if (loopEndX >= SCREEN_WIDTH) loopEndX = SCREEN_WIDTH - 1;

    int loopStartY = drawStartY; 
    if (loopStartY < 0) loopStartY = 0;
    int loopEndY = drawEndY;
    if (loopEndY >= SCREEN_HEIGHT) loopEndY = SCREEN_HEIGHT - 1;

    Uint32* texPixels = (Uint32*)currentSurf->pixels;

    float stepX = (float)textureWidth / (float)spriteScreenW;
    float stepY = (float)textureHeight / (float)spriteScreenH;

    float texPosX = (loopStartX - (projX - spriteScreenW / 2)) * stepX;
    
    for (int x = loopStartX; x <= loopEndX; ++x) {

        if (perpDist < zBuffer[x]) {

            int texX = (int)texPosX;
            if (texX >= textureWidth) texX = textureWidth - 1;

            float texPosY = (loopStartY - drawStartY) * stepY;

            for (int y = loopStartY; y <= loopEndY; ++y) {
                int texY = (int)texPosY;
                if (texY >= textureHeight) texY = textureHeight - 1;

                int index = texY * textureWidth + texX;

                int maxSize = (currentSurf->pitch / 4) * currentSurf->h;
            
                if (index >= 0 && index < maxSize) {
                    Uint32 color = texPixels[texY * textureWidth + texX];

                    if ((color >> 24) != 0) {
                        if(debugMode){
                        Uint8 a = (color >> 24) & 0xFF;
                        Uint8 r = (color >> 16) & 0xFF;
                        Uint8 g = (color >> 8)  & 0xFF;
                        Uint8 b = color         & 0xFF;

                        if (state == STATE_WALKING) {
                            // Tinte ROJO
                            r = 255; g = g / 2; b = b / 2; 
                        } 
                        else if (state == STATE_IDLE) {
                            // Tinte VERDE
                            g = 255; r = r / 2; b = b / 2; 
                        }
                        else if (state == STATE_ATTACKING) {
                            // Tinte AZUL
                            b = 255; r = r / 2; g = g / 2; 
                        }

                        color = (a << 24) | (r << 16) | (g << 8) | b;
                    }
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
            }
        }
    }
}

void Sprite::takeDamage(int amount) {
    if (state == STATE_DEAD || state == STATE_DYING) return;

    hp -= amount;
    if (hp <= 0) {
        hp = 0;
        state = STATE_DYING;
        animFrame = 0;
        animTimer = 0;

        if (!animDeath.empty()) {
            currentSurf = animDeath[0];
        }
    }
}

void Sprite::addDeathFrame(SDL_Surface* surf) {
    animDeath.push_back(surf);
}

void Sprite::setZOffset(float newZ) { zOffset = newZ;}