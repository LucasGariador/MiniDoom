#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <vector>   
#include "Sprite.h"


// Ancho y alto de la ventana
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int mapWidth = 15;
const int mapHeight = 15;

const float moveSpeed = 4.0; // velocidad de movimiento

const Uint8* keystates = SDL_GetKeyboardState(NULL);

// Config Player:
const float playerRadius = 0.20f;  // “grosor” del jugador
const float skin = 0.001f;  // evita quedar pegado al borde
float playerX = 7;
float playerY = 7;

// Config Minimap:
const int MINI_SCALE = 6;              // píxeles por celda del mapa
const int MINI_PADDING = 8;            // margen desde la esquina
const int MINI_RAYS_STEP = 2;          // cada cuántas columnas dibujar un rayo (sube perf)
bool showMinimap = true;               // toggle con tecla


std::vector<float> zBuffer(SCREEN_WIDTH); // global o miembro
void DrawMinimap(SDL_Renderer* renderer,
    int mapWidth, int mapHeight, int worldMap[][15], // ajusta segundo dim si tu mapa cambia
    float playerX, float playerY,
    float playerAngle,
    float fov,             // tu FOV en radianes
    int screenWidth, int screenHeight,
    const std::vector<float>& zBuffer); // opcional si querés pintar alcance de rayos

void MoveWithCollision(float& playerX, float& playerY,
    float dx, float dy,
    const int worldMap[mapHeight][mapWidth],
    float playerRadius, float skin); // declaración anticipada de la función lambda de colisión

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
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
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
	// Cargar texturas de pared
    SDL_Surface* wallSurface = SDL_LoadBMP("bricks.bmp"); // Carga una textura de pared desde un archivo BMP
	//cargar textura de sprite
	SDL_Surface* enemySurface = SDL_LoadBMP("zombieWB.bmp"); // Carga una textura de sprite desde un archivo BMP

    if (!enemySurface) {
        std::cerr << "Error al cargar la textura: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!wallSurface) {
        std::cerr << "Error al cargar la textura: " << SDL_GetError() << std::endl;
        return 1;
    }

	Uint32 colorKey = SDL_MapRGB(enemySurface->format, 255, 255, 255); // Color a transparentar (blanco)
	SDL_SetColorKey(enemySurface, SDL_TRUE, colorKey); // Establece el color clave para la transparencia

	Sprite enemy(5.0f, 9.0f, enemySurface); // Crea un sprite enemigo en la posición (5, 5)

    // 2. Crea una ventana con título y tamaño definidos
    SDL_Window* window = SDL_CreateWindow(
        "MiniDoom - Base",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
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
    float playerAngle = M_PI / 2.0f; //90 grados en radianes
	int playerDirection = 0; // 1 = mirando a la derecha, -1 = mirando a la izquierda, 0 = mirando al frente

    const float FOV = 60.0f * M_PI / 180.0f; // 60 grados en radianes
    const int numRays = SCREEN_WIDTH;       // Un rayo por píxel horizontal
    const float depth = 16.0f;              // Máxima distancia de renderizado
    


    // Bucle principal del juego
	Uint32 prevTicks = SDL_GetTicks();
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {

        std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());
        // Manejo de eventos (cerrar ventana, teclas, etc.)
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
                isRunning = false;
            }

            //Minimap toggle con tecla M
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            static bool mWasDown = false;
            bool mDown = keys[SDL_SCANCODE_M];
            if (mDown && !mWasDown) showMinimap = !showMinimap;
            mWasDown = mDown;
        }

		// 4. Calcular el tiempo transcurrido desde el último frame Delta Time
		Uint32 now = SDL_GetTicks();
		float deltaTime = (now - prevTicks) / 1000.0f; // convertir a segundos
		prevTicks = now;

        // 5. Lectura de teclado por estado

		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		float moveStep = moveSpeed * deltaTime; // velocidad de movimiento ajustada por el tiempo
		float rotStep = 0.05f; // velocidad de rotación en radianes


        // Movimiento del jugador con teclas WASD
        if (currentKeyStates[SDL_SCANCODE_W]) {
            float dx = cosf(playerAngle) * moveStep;
            float dy = sinf(playerAngle) * moveStep;
            MoveWithCollision(playerX, playerY, dx, dy, worldMap, playerRadius, skin);
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            float dx = -cosf(playerAngle) * moveStep;
            float dy = -sinf(playerAngle) * moveStep;
            MoveWithCollision(playerX, playerY, dx, dy, worldMap, playerRadius, skin);
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            float dx = -sinf(playerAngle) * moveStep;
            float dy = cosf(playerAngle) * moveStep;
            MoveWithCollision(playerX, playerY, dx, dy, worldMap, playerRadius, skin);
        }
        if (currentKeyStates[SDL_SCANCODE_A]) {
            float dx = sinf(playerAngle) * moveStep;
            float dy = -cosf(playerAngle) * moveStep;
            MoveWithCollision(playerX, playerY, dx, dy, worldMap, playerRadius, skin);
        }

		// Rotación del jugador con teclas de flecha
        if (currentKeyStates[SDL_SCANCODE_LEFT])  playerAngle -= rotStep;
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) playerAngle += rotStep;


        // 6. Limpiar la pantalla (color negro)
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // RGB + alpha
        SDL_RenderClear(renderer);

        int textureWidth = wallSurface->w;
        int textureHeight = wallSurface->h;
        Uint32* pixels = (Uint32*)wallSurface->pixels;
        int pitch = wallSurface->pitch / 4; // cantidad de Uint32s por fila


        // Raycasting
        for (int x = 0; x < SCREEN_WIDTH; x++) {
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



            float correctedDist = distanceToWall * cosf(rayAngle - playerAngle); // Corrección de la distancia para evitar distorsión por ángulo

            // Calcula la altura de la pared en pantalla
            int ceiling = std::max(0, (int)((SCREEN_HEIGHT / 2.0f) - SCREEN_HEIGHT / correctedDist));
            int floor = std::min(SCREEN_HEIGHT, SCREEN_HEIGHT - ceiling);

            // 1) Altura real de la pared en pantalla (sin recortar)
            int lineHeight = (int)(SCREEN_HEIGHT / correctedDist);

            // 2) Rango de dibujo teórico (sin recortar)
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;

            // 3) Clamp a la pantalla
            int yStart = std::max(0, drawStart);
            int yEnd = std::min(SCREEN_HEIGHT - 1, drawEnd);

            // 4) Paso de textura por píxel de pantalla, usando la altura REAL
            float step = (float)textureHeight / (float)lineHeight;

            // 5) Posición inicial en la textura: si la pared es más alta que la pantalla,
            //    empezamos a muestrear “abajo” en la textura (no desde 0)
            float texPos = (yStart - drawStart) * step;  // OJO: usa drawStart sin clamp

            for (int y = yStart; y <= yEnd; y++) {
                int textureY = (int)texPos;
                if (textureY < 0) textureY = 0;
                if (textureY >= textureHeight) textureY = textureHeight - 1;

                Uint32 color = pixels[textureY * pitch + textureX];
                Uint8 r, g, b; SDL_GetRGB(color, wallSurface->format, &r, &g, &b);

                // sombreado opcional...
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);

                texPos += step; // avanzar en la textura a ritmo constante
            }
            zBuffer[x] = correctedDist;
        }

        // Dibujar minimapa si está habilitado
        if (showMinimap) {
            DrawMinimap(renderer, mapWidth, mapHeight, worldMap,
                playerX, playerY, playerAngle, FOV,
                SCREEN_WIDTH, SCREEN_HEIGHT, zBuffer /* o {} */);
        }

		// 7. Dibujar sprites (enemigos, objetos, etc.)
		enemy.draw(renderer, zBuffer,SCREEN_WIDTH, SCREEN_HEIGHT, playerX, playerY, playerAngle, FOV);
	

        // Presentar en pantalla lo dibujado
        SDL_RenderPresent(renderer);

    }
    // 8. Liberar recursos antes de cerrar
    SDL_FreeSurface(wallSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void DrawMinimap(SDL_Renderer* renderer,
    int mapWidth, int mapHeight, int worldMap[][15], // ajusta segundo dim si tu mapa cambia
    float playerX, float playerY,
    float playerAngle,
    float fov,             // tu FOV en radianes
    int screenWidth, int screenHeight,
    const std::vector<float>& zBuffer) // opcional si querés pintar alcance de rayos
{
    // Origen del minimap en pantalla
    int originX = MINI_PADDING;
    int originY = MINI_PADDING;

    // Fondo del minimap
    SDL_Rect bg{ originX - 2, originY - 2, mapWidth * MINI_SCALE + 4, mapHeight * MINI_SCALE + 4 };
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 200);
    SDL_RenderFillRect(renderer, &bg);

    // Celdas
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            bool isWall = worldMap[y][x] != 0;
            SDL_Rect cell{
                originX + x * MINI_SCALE,
                originY + y * MINI_SCALE,
                MINI_SCALE, MINI_SCALE
            };
            if (isWall) {
                SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
                SDL_RenderFillRect(renderer, &cell);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255);
                SDL_RenderDrawRect(renderer, &cell);
            }
        }
    }

    // Rayos (opcional): dibujamos algunos para ver FOV y colisiones
    // Asumimos que tirás 1 rayo por columna en el render principal.
    // Dibujamos solo cada MINI_RAYS_STEP para ahorrar.
    int rays = screenWidth;
    for (int x = 0; x < rays; x += MINI_RAYS_STEP) {
        // Ángulo del rayo x (ajusta a tu cálculo real)
        float rayAngle = (playerAngle - fov * 0.5f) + (float)x / (float)rays * fov;

        // Punto final aproximado usando una distancia recortada (si no tenés zBuffer a mano)
        float maxD = 8.0f; // longitud visual del rayo en minimap
        float endX = playerX + cosf(rayAngle) * maxD;
        float endY = playerY + sinf(rayAngle) * maxD;

        // Si tenés zBuffer[x] con la distancia corregida, usalo:
        if (!zBuffer.empty() && x < (int)zBuffer.size()) {
            float d = std::min(zBuffer[x], maxD);
            endX = playerX + cosf(rayAngle) * d;
            endY = playerY + sinf(rayAngle) * d;
        }

        int sx = originX + (int)(playerX * MINI_SCALE);
        int sy = originY + (int)(playerY * MINI_SCALE);
        int ex = originX + (int)(endX * MINI_SCALE);
        int ey = originY + (int)(endY * MINI_SCALE);

        SDL_SetRenderDrawColor(renderer, 0, 200, 200, 180);
        SDL_RenderDrawLine(renderer, sx, sy, ex, ey);
    }

    // Jugador (punto)
    int px = originX + (int)(playerX * MINI_SCALE);
    int py = originY + (int)(playerY * MINI_SCALE);
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    SDL_Rect pRect{ px - 2, py - 2, 4, 4 };
    SDL_RenderFillRect(renderer, &pRect);

    // Dirección del jugador (flecha corta)
    int lookX = px + (int)(cosf(playerAngle) * 10);
    int lookY = py + (int)(sinf(playerAngle) * 10);
    SDL_SetRenderDrawColor(renderer, 255, 220, 0, 255);
    SDL_RenderDrawLine(renderer, px, py, lookX, lookY);
}

void MoveWithCollision(float& playerX, float& playerY,
    float dx, float dy,
    const int worldMap[mapHeight][mapWidth],
    float playerRadius, float skin) {
    // 1) mover en X
    float nextX = playerX + dx;
    int signX = (dx > 0.0f) ? 1 : -1;
    int testCellX = (int)(nextX + signX * playerRadius);
    int cellY = (int)(playerY);

    // chequeo de límites (ajusta mapWidth/mapHeight)
    if (testCellX >= 0 /* && testCellX < mapWidth && cellY >= 0 && cellY < mapHeight */) {
        if (worldMap[cellY][testCellX] == 0) {
            playerX = nextX;
        }
        else {
            // pegá el jugador al borde interno del pasillo
            float wallEdgeX = (signX > 0) ? (testCellX - playerRadius - skin)
                : (testCellX + 1 + playerRadius + skin);
            playerX = wallEdgeX;
        }
    }

    // 2) mover en Y
    float nextY = playerY + dy;
    int signY = (dy > 0.0f) ? 1 : -1;
    int testCellY = (int)(nextY + signY * playerRadius);
    int cellX = (int)(playerX);

    if (testCellY >= 0 /* && testCellY < mapHeight && cellX >= 0 && cellX < mapWidth */) {
        if (worldMap[testCellY][cellX] == 0) {
            playerY = nextY;
        }
        else {
            float wallEdgeY = (signY > 0) ? (testCellY - playerRadius - skin)
                : (testCellY + 1 + playerRadius + skin);
            playerY = wallEdgeY;
        }
    }
}