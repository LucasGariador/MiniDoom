#include "Sprite.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>


Sprite::Sprite(float px, float py, SDL_Surface* surf) : x(px), y(py), surface(surf), hp(100) {}

void Sprite::draw(SDL_Renderer* renderer,
    const std::vector<float>& zBuffer,
    int SCREEN_WIDTH, int SCREEN_HEIGHT,
    float playerX, float playerY,
    float playerAngle, float FOV)
{
    if (!surface) {
        std::cerr << "Error: Sprite surface is null.\n";
        return;
    }

    const int textureWidth = surface->w;
    const int textureHeight = surface->h;

    // Vector al sprite
    const float dx = x - playerX;
    const float dy = y - playerY;
    const float distance = sqrtf(dx * dx + dy * dy);
    if (distance <= 0.0001f) return;
    if (distance > 16.0f)    return; // culling por distancia (ajustable)

    // Ángulo sprite respecto al jugador
    float angleToSprite = atan2f(dy, dx);
    float angleDiff = angleToSprite - playerAngle;

    // Normalizar a [-PI, PI]
    while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;
    while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;

    // Si está fuera del FOV, no dibujar
    if (fabsf(angleDiff) > FOV * 0.5f) return;

    // Distancia perpendicular al plano de cámara para evitar "fisheye" en escala
    const float perpDist = distance * cosf(angleDiff);
    if (perpDist <= 0.0001f) return;

    // Proyección horizontal usando tangente (más estable que lineal)
    // x_pantalla = (W/2) * (1 + tan(Δ) / tan(FOV/2))
    const float halfW = SCREEN_WIDTH * 0.5f;
    const float halfH = SCREEN_HEIGHT * 0.5f;
    const float projX = halfW * (1.0f + tanf(angleDiff) / tanf(FOV * 0.5f));

    // Tamaño proyectado
    int spriteScreenH = (int)(SCREEN_HEIGHT / perpDist);
    if (spriteScreenH < 1) spriteScreenH = 1;

    int spriteScreenW = (int)((float)spriteScreenH * (float)textureWidth / (float)textureHeight);
    if (spriteScreenW < 1) spriteScreenW = 1;

    // Bounds en pantalla
    int drawStartX = (int)(projX - spriteScreenW * 0.5f);
    int drawEndX = (int)(projX + spriteScreenW * 0.5f);

    int drawStartY = (int)(halfH - spriteScreenH * 0.5f);
    int drawEndY = (int)(halfH + spriteScreenH * 0.5f);

    // Clamping a la ventana
    if (drawStartX < 0) drawStartX = 0;
    if (drawEndX >= SCREEN_WIDTH)  drawEndX = SCREEN_WIDTH - 1;
    if (drawStartY < 0) drawStartY = 0;
    if (drawEndY >= SCREEN_HEIGHT) drawEndY = SCREEN_HEIGHT - 1;

    // Acceso a píxeles de la textura
    Uint32* pixels32 = (Uint32*)surface->pixels;
    const int texPitch32 = surface->pitch / 4;

    // (Opcional) Si tu BMP ya trae blanco puro como fondo:
    Uint8 keyR = 255, keyG = 255, keyB = 255;

    // Dibujo columna por columna con prueba de z-buffer
    for (int screenX = drawStartX; screenX <= drawEndX; ++screenX) {

        // Si el sprite está detrás del muro en esta columna, saltar
        // Usamos la distancia perpendicular para comparar con zBuffer[screenX]
        if (screenX >= 0 && screenX < (int)zBuffer.size()) {
            if (perpDist >= zBuffer[screenX]) continue;
        }

        // Coordenada X en textura (0..textureWidth-1)
        float colFrac = (float)(screenX - (projX - spriteScreenW * 0.5f)) / (float)spriteScreenW;
        int texX = (int)(colFrac * textureWidth);
        if (texX < 0) texX = 0;
        if (texX >= textureWidth) texX = textureWidth - 1;

        // Recorro verticalmente sólo esta columna
        for (int screenY = drawStartY; screenY <= drawEndY; ++screenY) {

            // Coordenada Y en textura (0..textureHeight-1)
            float rowFrac = (float)(screenY - (halfH - spriteScreenH * 0.5f)) / (float)spriteScreenH;
            int texY = (int)(rowFrac * textureHeight);
            if (texY < 0) texY = 0;
            if (texY >= textureHeight) texY = textureHeight - 1;

            // Leer color del texel
            Uint32 color = pixels32[texY * texPitch32 + texX];

            Uint8 r, g, b, a = 255;
            SDL_GetRGBA(color, surface->format, &r, &g, &b, &a);

            // Transparencia:
            // 1) Si hay canal alfa y es 0, lo ignoramos
            if (surface->format->Amask && a == 0) continue;

            // 2) Colorkey manual (blanco puro de fondo)
            if (r == keyR && g == keyG && b == keyB) continue;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawPoint(renderer, screenX, screenY);
        }
    }
}

