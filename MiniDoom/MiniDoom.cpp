#include <SDL.h>
#include <iostream>
#include <algorithm>

// Ancho y alto de la ventana
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int mapWidth = 15;
const int mapHeight = 15;

const float moveSpeed = 0.1f; // velocidad de movimiento

const Uint8* keystates = SDL_GetKeyboardState(NULL);

// Mapa simple: 1 = pared, 0 = espacio libre
int worldMap[mapHeight][mapWidth] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1},
    {1,0,0,1,1,1,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,1,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};


int main(int argc, char* argv[]) {
    // 1. Inicializa SDL (en este caso, solo el sistema de video)
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

	SDL_Surface* wallSurface = SDL_LoadBMP("wall.bmp"); // Carga una textura de pared desde un archivo BMP

    if (!wallSurface) {
        std::cerr << "Error al cargar la textura: " << SDL_GetError() << std::endl;
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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::cerr << "Error al crear el renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    float playerX = 7.5f;
    float playerY = 7.5f;
    float playerAngle = M_PI / 2.0f;  // 45 grados en radianes


    const float FOV = 60.0f * M_PI / 180.0f; // 60 grados en radianes
    const int numRays = SCREEN_WIDTH;       // Un rayo por píxel horizontal
    const float depth = 16.0f;              // Máxima distancia de renderizado


	// Bucle principal del juego
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        // Manejo de eventos (cerrar ventana, teclas, etc.)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
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

			// Movimiento del jugador con teclas WASD
            if (keystates[SDL_SCANCODE_W]) {
                float newX = playerX + cos(playerAngle) * moveSpeed;
                float newY = playerY + sin(playerAngle) * moveSpeed;

                // Colisiones
                if (worldMap[(int)newY][(int)newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                }
            }
            if (keystates[SDL_SCANCODE_S]) {
                float newX = playerX - cos(playerAngle) * moveSpeed;
                float newY = playerY - sin(playerAngle) * moveSpeed;
                if (worldMap[(int)newY][(int)newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                }
            }
            if (keystates[SDL_SCANCODE_D]) {
                float newX = playerX - sin(playerAngle) * moveSpeed;
                float newY = playerY + cos(playerAngle) * moveSpeed;
                if (worldMap[(int)newY][(int)newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                }
            }
            if (keystates[SDL_SCANCODE_A]) {
                float newX = playerX + sin(playerAngle) * moveSpeed;
                float newY = playerY - cos(playerAngle) * moveSpeed;
                if (worldMap[(int)newY][(int)newX] == 0) {
                    playerX = newX;
                    playerY = newY;
                }
            }

        }

        // 6. Limpiar la pantalla (color negro)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB + alpha
        SDL_RenderClear(renderer);

        int textureWidth = wallSurface->w;
        int textureHeight = wallSurface->h;
        Uint32* pixels = (Uint32*)wallSurface->pixels;
        int pitch = wallSurface->pitch / 4; // cantidad de Uint32s por fila


        // Aquí en el futuro se dibujará el raycasting, jugador, etc.
        for (int x = 0; x < SCREEN_WIDTH; x += 2) {
            // Calcula el ángulo del rayo
            float rayAngle = (playerAngle - FOV / 2.0f) + ((float)x / (float)(SCREEN_WIDTH)) * FOV;

            float distanceToWall = 0.0f;
            bool hitWall = false;

            float eyeX = cos(rayAngle); // dirección X del rayo
            float eyeY = sin(rayAngle); // dirección Y del rayo

            // Avanza paso a paso hasta chocar con una pared o alcanzar la distancia máxima
            while (!hitWall && distanceToWall < depth) {
                distanceToWall += 0.01f;

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
            int hitBlockX = (int)(playerX + eyeX * distanceToWall);
            int hitBlockY = (int)(playerY + eyeY * distanceToWall);

            float blockMidX = hitBlockX + 0.5f;
            float blockMidY = hitBlockY + 0.5f;

            float dx = (playerX + eyeX * distanceToWall) - blockMidX;
            float dy = (playerY + eyeY * distanceToWall) - blockMidY;

            bool verticalHit = fabs(dx) > fabs(dy);


            float hitX = playerX + eyeX * distanceToWall;
            float hitY = playerY + eyeY * distanceToWall;

            float wallX;

            if (verticalHit) {
                wallX = hitY - floor(hitY);
                if (eyeX > 0) wallX = 1.0f - wallX;
            }
            else {
                wallX = hitX - floor(hitX);
                if (eyeY < 0) wallX = 1.0f - wallX;
            }


            int textureX = (int)(wallX * textureWidth);
            if (textureX < 0) textureX = 0;
            if (textureX >= textureWidth) textureX = textureWidth - 1;



			float correctedDist = distanceToWall * cos(rayAngle - playerAngle); // Corrección de la distancia para evitar distorsión por ángulo

            // Calcula la altura de la pared en pantalla
            int ceiling = std::max(0, (int)((SCREEN_HEIGHT / 2.0f) - SCREEN_HEIGHT / correctedDist));
            int floor = std::min(SCREEN_HEIGHT, SCREEN_HEIGHT - ceiling);

            for (int y = ceiling; y < floor; y++) {
                // Mapeo vertical entre pantalla y textura
                float texYRatio = (float)(y - ceiling) / (floor - ceiling);
                int textureY = (int)(texYRatio * textureHeight);

                Uint32 color = pixels[textureY * pitch + textureX];

                // Extraer RGB del color
                Uint8 r, g, b;
                SDL_GetRGB(color, wallSurface->format, &r, &g, &b);

                // Sombrear según distancia (opcional)
                float brightness = 1.0f - std::min(correctedDist / depth, 1.0f);
                r *= brightness;
                g *= brightness;
                b *= brightness;

                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }

        }

        // 7. Presentar en pantalla lo dibujado
        SDL_RenderPresent(renderer);
    }

    // 8. Liberar recursos antes de cerrar
	SDL_FreeSurface(wallSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
