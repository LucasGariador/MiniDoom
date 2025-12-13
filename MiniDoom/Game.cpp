#include "Game.h"

Game::Game()
    : playerX(0), playerY(0), playerAngle(0), FOV(0), // Valores por defecto seguros
    isRunning(false), window(nullptr), renderer(nullptr), ceilH(0), ceilW(0),
    wallSurface(nullptr), floorPixels(nullptr), zBuffer(), font(nullptr),
    crosshairTexture(nullptr), textureEnemyIdle(nullptr), screenTexture(nullptr),
    screenBuffer(nullptr), ceilPixels(nullptr), floorW(0), floorH(0), screenWidth(0), screenHeight(0),
	playerIsMoving(false), deltaTime(0.0f), mouseSensitivity(0.003f), moveSpeed(4.0f), crossH(0), crossW(0), 
	crosshairSurf(nullptr), fireballTex(nullptr), floorSurface(nullptr), wallPixels(nullptr)
{
    //Nada
}

Game::~Game() {
    // Simplemente llamamos a clean
//    clean();
}

bool Game::init(const char* title, int width, int height) {
    screenWidth = width;
    screenHeight = height;

    playerX = 2.5f;            // Posición inicial X (mitad de la celda 3)
    playerY = 7.5f;            // Posición inicial Y
	playerAngle = 0.0f; //ARRIBA(Norte) 3 * M_PI / 2 // Derecha(Este) 0 // Abajo(Sur) M_PI / 2 // Izquierda(Oeste) M_PI

    FOV = 60.0f * (M_PI / 180.0f); // 60 grados convertidos a radianes

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    // Inicializa SDL_image para soportar PNG
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "Error iniciando SDL_image: " << IMG_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("Error cargando SDL_ttf: %s\n", TTF_GetError());
        return -1;
    }

    font = TTF_OpenFont("Jacquard12-Regular.ttf", 24);
    if (!font) {
        printf("Error cargando fuente: %s\n", TTF_GetError());
        return -1;
    }

    //    Crea una ventana con título y tamaño definidos
        window = SDL_CreateWindow(
            "MiniDoom - Base",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            screenWidth, screenHeight,
            SDL_WINDOW_SHOWN
        );

    if (!window) {
        std::cerr << "Error al crear la ventana: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Crea un renderizador asociado a esa ventana (para dibujar)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        std::cerr << "Error al crear el renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Inicializar Buffer
    screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    screenBuffer = new Uint32[width * height];
    zBuffer.resize(width);

    // Cargar Texturas
        // Cargar texturas de pared
    wallSurface = Utils::LoadTexture("wall_bricks_512x512.png"); // Carga una textura de pared desde un archivo
    //Suelo
    SDL_Surface* floorSurf = Utils::LoadTexture("floor_stone_512x512.png");
    SDL_Surface* ceilSurf = Utils::LoadTexture("floor_stone_512x512.png");
    floorPixels = (Uint32*)floorSurf->pixels;
    ceilPixels = (Uint32*)ceilSurf->pixels;
    floorW = floorSurf->w;
    floorH = floorSurf->h;

    ceilW = ceilSurf->w;
    ceilH = ceilSurf->h;
    //cargar textura de sprite
    //Weapon 
    Weapon playerStaff;
    playerStaff.init(renderer, "weapon_staff_idle.png", "staff_fire.png");
    //Projectile texture
    fireballTex = Utils::LoadTexture("fireball2.png");
    //Enemys
    textureEnemyIdle = Utils::LoadTexture("ogre_idle.png");

    // Cargar secuencia de muerte
    textureEnemyDie.push_back(Utils::LoadTexture("ogre_die1.png"));
    textureEnemyDie.push_back(Utils::LoadTexture("ogre_die2.png"));
    textureEnemyDie.push_back(Utils::LoadTexture("ogre_die3.png"));
    textureEnemyDie.push_back(Utils::LoadTexture("ogre_dead.png"));

    if (!textureEnemyIdle) {
        std::cerr << "Error al cargar la textura: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!wallSurface) {
        std::cerr << "Error al cargar la textura: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (!playerStaff.textureIdle || !playerStaff.textureFire) {
        std::cerr << "Error al cargar la textura del arma: " << SDL_GetError() << std::endl;
        return 1;
    }

    crosshairSurf = Utils::LoadTexture("crosshair.png");

    if (!crosshairSurf) {
        std::cout << "Error cargando crosshair" << std::endl; // Manejo de error básico
    }

    crosshairTexture = SDL_CreateTextureFromSurface(renderer, crosshairSurf);

    crossW = crosshairSurf->w;
    crossH = crosshairSurf->h;
    SDL_FreeSurface(crosshairSurf);

    loadLevel();
    isRunning = true;

    return true;
}

void Game::handleEvents() {

    SDL_SetRelativeMouseMode(SDL_TRUE);

    std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<float>::infinity());

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) isRunning = false;

        // Disparos y Mouse Look
        if (event.type == SDL_MOUSEMOTION) {
            playerAngle += event.motion.xrel * mouseSensitivity;
            // Normalización segura del ángulo
            if (playerAngle < 0) playerAngle += 2 * M_PI;
            if (playerAngle > 2 * M_PI) playerAngle -= 2 * M_PI;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (ammo > 0) {
                    playerStaff.shoot(); // Animación del báculo
                    // CREAR PROYECTIL
                    // Lo movemos un poquito adelante del jugador (0.5f) para que no nazca dentro de ti
                    float spawnX = playerX + cosf(playerAngle) * 0.5f;
                    float spawnY = playerY + sinf(playerAngle) * 0.5f;

                    Projectile* p = new Projectile(spawnX, spawnY, playerAngle, fireballTex);
                    projectiles.push_back(p);
                    ammo--;
                }
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                health = std::max(health - 20, 0);
            }
        }
    }

}

void Game::update() {
    // Calcular DeltaTime (puedes hacer variables miembros currentTick y lastTick)
    static Uint32 prevTicks = SDL_GetTicks();
    Uint32 now = SDL_GetTicks();
    deltaTime = (now - prevTicks) / 1000.0f;
    prevTicks = now;

    // Movimiento Jugador (WASD)
    const Uint8* state = SDL_GetKeyboardState(NULL);
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    float moveStep = moveSpeed * deltaTime;

    // Cálculo de vectores de movimiento (optimizamos calculando seno/coseno una sola vez)
    float cosA = cosf(playerAngle);
    float sinA = sinf(playerAngle);

    if (currentKeyStates[SDL_SCANCODE_LSHIFT]) moveStep *= 1.5f; else moveStep /= 1.5f;

    if (currentKeyStates[SDL_SCANCODE_W]) MoveWithCollision(playerX, playerY, cosA * moveStep, sinA * moveStep, worldMap, playerRadius, skin);
    if (currentKeyStates[SDL_SCANCODE_S]) MoveWithCollision(playerX, playerY, -cosA * moveStep, -sinA * moveStep, worldMap, playerRadius, skin);
    if (currentKeyStates[SDL_SCANCODE_D]) MoveWithCollision(playerX, playerY, -sinA * moveStep, cosA * moveStep, worldMap, playerRadius, skin);
    if (currentKeyStates[SDL_SCANCODE_A]) MoveWithCollision(playerX, playerY, sinA * moveStep, -cosA * moveStep, worldMap, playerRadius, skin);

    playerIsMoving = false;
    if (currentKeyStates[SDL_SCANCODE_W] ||
        currentKeyStates[SDL_SCANCODE_S] ||
        currentKeyStates[SDL_SCANCODE_A] ||
        currentKeyStates[SDL_SCANCODE_D])
    {
        playerIsMoving = true;
    }

    // Actualizar Proyectiles
    for (int i = 0; i < projectiles.size(); i++) {
        Projectile* p = projectiles[i];

        // Mover y chequear paredes
        p->update(deltaTime, mapWidth, mapHeight, worldMap);

        // Chequear colisión con el Enemigo (Si está vivo)
        for (Sprite* s : sprites) {
            if (p->active && !s->isDead) {
                // Distancia simple (Pitágoras) entre proyectil y enemigo
                float distSq = (p->x - s->x) * (p->x - s->x) + (p->y - s->y) * (p->y - s->y);

                // Si la distancia es menor a 0.5 (radio de impacto), ¡BOOM!
                if (distSq < 0.5f * 0.5f) {
                    s->takeDamage(35);
                    p->active = false; // El proyectil explota al impactar
                    std::cout << "Impacto! HP: " << s->hp << std::endl;
                }
            }
        }
    }

    for (Sprite* s : sprites) {
        s->update(deltaTime); // <--- IMPORTANTE
    }

    // Limpieza: Eliminar proyectiles inactivos de la memoria
    // (Esto es una forma moderna de borrar elementos de un vector en C++)
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](Projectile* p) {
            if (!p->active) {
                delete p; // Liberar memoria
                return true;
            }
            return false;
        }), projectiles.end());
}

void Game::render() {
    // 1. Limpiar Buffer (memset)
    memset(screenBuffer, 0, screenWidth * screenHeight * 4);

    // 2. Raycasting (Paredes, Suelo, Techo)
    renderWorld(); // <--- Mueve el bucle gigante FOR x=0... a esta función privada

    // 3. Actualizar Textura SDL
    SDL_UpdateTexture(screenTexture, NULL, screenBuffer, screenWidth * 4);

    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);

    // 4. Sprites y Arma (Encima del buffer)
    renderSprites();

    renderUI();

    // 5. Presentar
    SDL_RenderPresent(renderer);
}

void Game::renderWorld() {
    // Aquí va el bucle gigante de Raycasting que tenías en MiniDoom.cpp
    // Reemplaza SCREEN_WIDTH/HEIGHT por screenWidth/Height
    // Usa screenBuffer para pintar píxeles
    // Guarda distancias en zBuffer
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Datos de textura para acceso rápido
    int textureWidth = wallSurface->w;
    int textureHeight = wallSurface->h;
    Uint32* wallPixels = (Uint32*)wallSurface->pixels;
    int wallPitch = wallSurface->pitch / 4;
    // --- ALGORITMO DDA RAYCASTING ---
    for (int x = 0; x < screenWidth; x++) {

        // 1. Calcular dirección del rayo
        float cameraX = 2 * x / (float)screenWidth - 1; // Coordenada en espacio de cámara (-1 a 1)
        float rayDirX = cosf(playerAngle) + cosf(playerAngle + M_PI / 2) * tanf(FOV / 2) * cameraX; // Vector plano cámara simplificado
        float rayDirY = sinf(playerAngle) + sinf(playerAngle + M_PI / 2) * tanf(FOV / 2) * cameraX;

        // Método más simple si usas FOV fijo:
        float rayAngle = (playerAngle - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;
        float eyeX = cosf(rayAngle);
        float eyeY = sinf(rayAngle);

        // Posición en el mapa (enteros)
        int mapX = (int)playerX;
        int mapY = (int)playerY;

        // Longitud del rayo desde la posición actual al siguiente lado x o y
        float sideDistX, sideDistY;

        // Longitud del rayo de un lado x/y al siguiente x/y
        // Se usa valor absoluto y una guardia para evitar división por cero
        float deltaDistX = (eyeX == 0) ? 1e30 : std::abs(1.0f / eyeX);
        float deltaDistY = (eyeY == 0) ? 1e30 : std::abs(1.0f / eyeY);

        float perpWallDist;

        // Dirección de paso y sideDist inicial
        int stepX, stepY;
        int hit = 0;
        int side; // 0 para NS, 1 para EO

        if (eyeX < 0) {
            stepX = -1;
            sideDistX = (playerX - mapX) * deltaDistX;
        }
        else {
            stepX = 1;
            sideDistX = (mapX + 1.0f - playerX) * deltaDistX;
        }
        if (eyeY < 0) {
            stepY = -1;
            sideDistY = (playerY - mapY) * deltaDistY;
        }
        else {
            stepY = 1;
            sideDistY = (mapY + 1.0f - playerY) * deltaDistY;
        }

        // 2. Ejecutar DDA
        while (hit == 0) {
            // Saltar al siguiente cuadro del mapa, sea en dirección x o y
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            // Chequear si el rayo se salió del mapa (EVITA CRASH)
            if (mapX < 0 || mapX >= mapWidth || mapY < 0 || mapY >= mapHeight) {
                hit = 1;
                perpWallDist = depth; // Pared lejana falsa
            }
            // Chequear si golpeó pared
            else if (worldMap[mapY][mapX] > 0) {
                hit = 1;
            }
        }

        // 3. Calcular distancia corregida (Fisheye fix automático del DDA)
        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
        else           perpWallDist = (sideDistY - deltaDistY);

        // Guardar en ZBuffer para los sprites
        zBuffer[x] = perpWallDist;

        float wallX;
        if (side == 0) wallX = playerY + perpWallDist * rayDirY;
        else           wallX = playerX + perpWallDist * rayDirX;
        wallX -= floor((wallX));

        int texX = (int)(wallX * (float)textureWidth);

        // 3. CORREGIR INVERSIÓN
        if (side == 0 && rayDirX > 0) texX = textureWidth - texX - 1;
        if (side == 1 && rayDirY < 0) texX = textureWidth - texX - 1;

        // 4. Calcular altura de línea
        int lineHeight = (int)(screenHeight / perpWallDist);

        // Calcular start y end de dibujo
        int drawStart = -lineHeight / 2 + screenHeight / 2;
        int drawEnd = lineHeight / 2 + screenHeight / 2;

        // Clamp para texturizado
        int drawStartClamped = std::max(0, drawStart);
        int drawEndClamped = std::min(screenHeight - 1, drawEnd);

        if (drawEnd < 0) drawEnd = screenHeight;

        // --- A. DIBUJAR PAREDES (EN EL BUFFER) ---
        float step = 1.0f * textureHeight / lineHeight;
        float texPos = (drawStartClamped - screenHeight / 2 + lineHeight / 2) * step;

        for (int y = drawStartClamped; y < drawEndClamped; y++) {
            int texY = (int)texPos & (textureHeight - 1);
            texPos += step;
            Uint32 color = wallPixels[texY * wallPitch + texX];
            if (side == 1) color = (color >> 1) & 8355711;

            // IMPORTANTE: Escribir directo en memoria, no usar SDL_RenderDrawPoint
            screenBuffer[y * screenWidth + x] = color;
        }

        // --- B. CÁLCULO DE SUELO Y TECHO ---

        // 1. Coordenadas exactas del suelo al pie de la pared
        float floorXWall, floorYWall;

        // Dependiendo de la dirección del rayo y lado de impacto, sabemos dónde "toca" el suelo
        // Usamos 'rayDirX' y 'rayDirY' que calculaste al principio del bucle
        if (side == 0 && rayDirX > 0) {
            floorXWall = mapX;
            floorYWall = mapY + wallX;
        }
        else if (side == 0 && rayDirX < 0) {
            floorXWall = mapX + 1.0f;
            floorYWall = mapY + wallX;
        }
        else if (side == 1 && rayDirY > 0) {
            floorXWall = mapX + wallX;
            floorYWall = mapY;
        }
        else {
            floorXWall = mapX + wallX;
            floorYWall = mapY + 1.0f;
        }

        float distWall = perpWallDist;
        float distPlayer = 0.0f; // Distancia en el centro (jugador)

        // 2. Bucle desde el pie de la pared hasta el final de la pantalla
        for (int y = drawEndClamped + 1; y < screenHeight; y++) {

            // Fórmula mágica de proyección: Convierte píxel Y en distancia Z
            float currentDist = screenHeight / (2.0f * y - screenHeight);

            // Interpolación lineal (Weighting)
            // Cuanto más cerca de la pared, weight se acerca a 1. 
            // Cuanto más cerca del jugador (abajo pantalla), weight se acerca a 0.
            float weight = (currentDist - distPlayer) / (distWall - distPlayer);

            // Coordenada exacta en el mapa para este píxel
            float currentFloorX = weight * floorXWall + (1.0f - weight) * playerX;
            float currentFloorY = weight * floorYWall + (1.0f - weight) * playerY;

            // Convertir a coordenadas de textura
            // El operador % (módulo) no funciona bien con floats negativos, así que casteamos a int
            // y aseguramos que sea positivo multiplicando por el tamaño.
            int floorTexX = (int)(currentFloorX * floorW) % floorW;
            int floorTexY = (int)(currentFloorY * floorH) % floorH;

            // --- PINTAR SUELO ---
            Uint32 colorFloor = floorPixels[floorTexY * floorW + floorTexX];
            // Aplicar un poco de sombra por distancia (opcional)
            // colorFloor = (colorFloor >> 1) & 8355711; 
            screenBuffer[y * screenWidth + x] = colorFloor;

            // --- PINTAR TECHO (SIMETRÍA) ---
            // El techo es el espejo del suelo. Usamos (SCREEN_HEIGHT - y - 1)
            int ceilingY = screenHeight - y - 1;
            if (ceilingY >= 0) {
                Uint32 colorCeil = ceilPixels[floorTexY * ceilW + floorTexX];
                screenBuffer[ceilingY * screenWidth + x] = colorCeil;
            }
        }
    }

    // --- ACTUALIZAR GPU ---
    // Una vez terminado todo el cálculo, enviamos la imagen completa
    SDL_UpdateTexture(screenTexture, NULL, screenBuffer, screenWidth * sizeof(Uint32));
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
}

void Game::renderSprites() {

    for (Sprite* s : sprites) {
        if (!s->isDead) { // Solo si está vivo
            s->draw(renderer, zBuffer, screenWidth, screenHeight, playerX, playerY, playerAngle, FOV);
        }
    }

    for (Projectile* p : projectiles) {
        // Usamos la misma función de dibujo que el enemigo
        p->spriteVis->draw(renderer, zBuffer, screenWidth, screenHeight, playerX, playerY, playerAngle, FOV);
    }

    playerStaff.draw(renderer, screenWidth, screenHeight, deltaTime, playerIsMoving);

}

void Game::renderUI() {
    if (showMinimap) {
        DrawMinimap(renderer, mapWidth, mapHeight, worldMap,
            playerX, playerY, playerAngle, FOV,
            screenWidth, screenHeight, zBuffer);
    }
    if (showUI) {
        // DIBUJAR UI

        // A. Barra de vida
        // Actualizar ancho según vida
        healthBarFront.w = (int)((health / 100.0f) * 200);

        // Dibujar Fondo
        SDL_SetRenderDrawColor(renderer, 50, 0, 0, 255);
        SDL_RenderFillRect(renderer, &healthBarBack);
        // Dibujar Vida Actual
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &healthBarFront);

        // B. Texto de Munición
        SDL_Color textColor = { 255, 255, 0 }; // Amarillo
        char ammoBuffer[20];
        sprintf_s(ammoBuffer, "Ammo: %d", ammo);

        // Crear superficie -> Crear textura -> Renderizar
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, ammoBuffer, textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textW = 0, textH = 0;
        SDL_QueryTexture(textTexture, NULL, NULL, &textW, &textH);
        SDL_Rect textRect = { 700, 550, textW, textH }; // Posición fija (x:20, y:60)

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
    SDL_Rect crossRect;
    crossRect.x = (screenWidth - crossW) / 2;
    crossRect.y = (screenHeight - crossH) / 2;
    crossRect.w = crossW;
    crossRect.h = crossH;

    SDL_RenderCopy(renderer, crosshairTexture, NULL, &crossRect);
}

void Game::checkShooting(Sprite* enemy, float playerX, float playerY, float playerAngle, const std::vector<float>& zBuffer, int SCREEN_WIDTH) {
    if (enemy->isDead) return;

    // 1. Vector hacia el enemigo
    float dx = enemy->x - playerX;
    float dy = enemy->y - playerY;
    float distToEnemy = sqrtf(dx * dx + dy * dy);

    // 2. Calcular ángulo hacia el enemigo
    float angleToEnemy = atan2f(dy, dx) - playerAngle;

    // Normalizar ángulo (-PI a PI)
    while (angleToEnemy < -M_PI) angleToEnemy += 2 * M_PI;
    while (angleToEnemy > M_PI) angleToEnemy -= 2 * M_PI;

    // 3. ¿Estamos apuntando al enemigo?
    // Si el ángulo es muy pequeño (ej. 0.1 radianes), lo tenemos en la mira.
    // También verificamos que esté cerca (rango del arma).
    if (fabs(angleToEnemy) < 0.15f && distToEnemy < 10.0f) {

        // 4. Muro en el medio (Oclusión)
        // Miramos el ZBuffer en el centro de la pantalla (Screen Width / 2)
        // Si la pared está más cerca que el enemigo, fallamos el tiro.
        float wallDist = zBuffer[SCREEN_WIDTH / 2];

        if (distToEnemy < wallDist) {
            enemy->takeDamage(50); // ¡PUM! 50 de daño
        }
    }
}

void Game::MoveWithCollision(float& playerX, float& playerY,
    float dx, float dy,
    const int worldMap[mapHeight][mapWidth],
    float playerRadius, float skin) {
    // 1) mover en X
    float nextX = playerX + dx;
    int signX = (dx > 0.0f) ? 1 : -1;
    int testCellX = (int)(nextX + signX * playerRadius);
    int cellY = (int)(playerY);

    // chequeo de límites (ajusta mapWidth/mapHeight)
    if (testCellX >= 0 && testCellX < mapWidth && cellY >= 0 && cellY < mapHeight) {
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

    if (testCellY >= 0 && testCellY < mapHeight && cellX >= 0 && cellX < mapWidth) {
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

void Game::loadLevel() {
    // Primero, limpiamos cualquier sprite viejo (si reinicias nivel)
    for (auto s : sprites) delete s;
    sprites.clear();

    // Recorremos todo el mapa
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {

            int cellType = worldMap[y][x];

            // SI ES UN ENEMIGO (Código 9)
            if (cellType == 9) {
                // Crear el sprite en el CENTRO de la celda (x + 0.5f)
                Sprite* s = new Sprite(x + 0.5f, y + 0.5f, textureEnemyIdle);

                for (auto surf : textureEnemyDie) {
                    s->addDeathFrame(surf);
                }
                sprites.push_back(s);

                // IMPORTANTE: Borrar el 9 del mapa para que sea suelo transitable
                worldMap[y][x] = 0;
            }

            // SI ES EL JUGADOR (Código 5)
            else if (cellType == 5) {
                playerX = x + 0.5f;
                playerY = y + 0.5f;
                worldMap[y][x] = 0; // Limpiar celda
            }

            // Puedes añadir más 'else if' para objetos, munición, etc.
        }
    }
}

void Game::DrawMinimap(SDL_Renderer* renderer,
    int mapWidth, int mapHeight, int worldMap[][30], // ajusta segundo dim si tu mapa cambia
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

void Game::clean() {
    // Liberar recursos
    if (screenBuffer) {
        delete[] screenBuffer;
        screenBuffer = nullptr;
    }
    if (screenTexture) {
        SDL_DestroyTexture(screenTexture);
        screenTexture = nullptr;
    }
    if (font != nullptr) {
        TTF_CloseFont(font);
        font = nullptr; // Buena práctica: anularlo después de cerrar
    }
    if (crosshairTexture) {
        SDL_DestroyTexture(crosshairTexture);
    }

    for (SDL_Surface* surf : textureEnemyDie) {
        SDL_FreeSurface(surf);
    }
    textureEnemyDie.clear();

    if (textureEnemyIdle) {
        SDL_FreeSurface(textureEnemyIdle);
        textureEnemyIdle = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    for (auto p : projectiles) {
        delete p;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    std::cout << "Juego cerrado correctamente." << std::endl;
}