#include <SDL.h>
#include <iostream>
#include <algorithm>

// Ancho y alto de la ventana
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int mapWidth = 8;
const int mapHeight = 8;

// Mapa simple: 1 = pared, 0 = espacio libre
int worldMap[mapHeight][mapWidth] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,0,1},
    {1,0,0,0,1,0,0,1},
    {1,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1}
};


int main(int argc, char* argv[]) {
    // 1. Inicializa SDL (en este caso, solo el sistema de video)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Crea una ventana con título y tamaño definidos
    SDL_Window* window = SDL_CreateWindow(
        "MiniDoom - Base",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Error al crear la ventana: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Crea un renderizador asociado a esa ventana (para dibujar)
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Error al crear el renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    float playerX = 3.5f;
    float playerY = 3.5f;
    float playerAngle = M_PI / 2.0f;  // 45 grados en radianes


    const float FOV = 60.0f * M_PI / 180.0f; // 60 grados en radianes
    const int numRays = SCREEN_WIDTH;       // Un rayo por píxel horizontal
    const float depth = 16.0f;              // Máxima distancia de renderizado


    // 4. Bucle principal del juego
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        // 5. Manejo de eventos (cerrar ventana, teclas, etc.)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    playerAngle -= 0.1f; // gira a la izquierda
                    break;
                case SDLK_RIGHT:
                    playerAngle += 0.1f; // gira a la derecha
                    break;
                }
                if (playerAngle < 0) playerAngle += 2 * M_PI;
                if (playerAngle > 2 * M_PI) playerAngle -= 2 * M_PI;

            }

        }

        // 6. Limpiar la pantalla (color negro)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB + alpha
        SDL_RenderClear(renderer);

        // Aquí en el futuro se dibujará el raycasting, jugador, etc.
        for (int x = 0; x < numRays; x++) {
            // Calcula el ángulo del rayo
            float rayAngle = (playerAngle - FOV / 2.0f) + ((float)x / (float)numRays) * FOV;

            float distanceToWall = 0.0f;
            bool hitWall = false;

            float eyeX = cos(rayAngle); // dirección X del rayo
            float eyeY = sin(rayAngle); // dirección Y del rayo

            // Avanza paso a paso hasta chocar con una pared o alcanzar la distancia máxima
            while (!hitWall && distanceToWall < depth) {
                distanceToWall += 0.1f;

                int testX = (int)(playerX + eyeX * distanceToWall);
                int testY = (int)(playerY + eyeY * distanceToWall);

                // Si salimos del mapa, contamos como pared
                if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight) {
                    hitWall = true;
                    distanceToWall = depth;
                }
                else if (worldMap[testY][testX] == 1) {
                    hitWall = true;
                }
            }

            // Calcula la altura de la pared en pantalla
            int ceiling = std::max(0, (int)((SCREEN_HEIGHT / 2.0f) - SCREEN_HEIGHT / distanceToWall));
            int floor = std::min(SCREEN_HEIGHT, SCREEN_HEIGHT - ceiling);

            // Cambia el color según la distancia (más oscuro si está lejos)
            Uint8 shade = (Uint8)(255 - (distanceToWall / depth) * 255);
            SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);

            // Dibuja una línea vertical como “pared”
            SDL_RenderDrawLine(renderer, x, ceiling, x, floor);
        }

        // 7. Presentar en pantalla lo dibujado
        SDL_RenderPresent(renderer);
    }

    // 8. Liberar recursos antes de cerrar
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
