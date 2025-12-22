#include "Enemy.h"
#include "Projectile.h"

void Enemy::updateAI(float deltaTime, float playerX, float playerY, int& playerHP, std::vector<Projectile*>& projectiles, SDL_Surface* bulletTex, const int worldMap[30][30]) {
    // 1. Si está muerto, solo animar muerte y salir
    if (state == STATE_DEAD || state == STATE_DYING) {
        update(deltaTime); // Llama al update base para la animación de muerte
        return;
    }
	this->mapHeight = mapHeight;
	this->mapWidth = mapWidth;
    // 2. Calcular distancia al jugador
    float dx = playerX - x;
    float dy = playerY - y;
    float dist = sqrt(dx * dx + dy * dy);
    float angleToPlayer = atan2(dy, dx);

    // 3. Temporizador de ataque
    if (attackTimer > 0) attackTimer -= deltaTime;

    // ----------------------------------------------------
    // LÓGICA DE MOVIMIENTO Y ATAQUE
    // ----------------------------------------------------
    canSeePlayer = CheckLineOfSight(x, y, playerX, playerY, worldMap);

    if(dist > aggroRange) {
        // Fuera de rango de agresión: Quedarse quieto
        state = STATE_IDLE;
        hasSeenThePlayer = canSeePlayer;
        update(deltaTime);
        return;
	}

    // Si estamos lejos, nos movemos hacia el jugador
    if (dist > attackRange && hasSeenThePlayer) {
        state = STATE_ATTACKING; // O caminando

        // Movimiento simple hacia el jugador
        float speed = (type == TYPE_BOSS) ? 1.0f : 2.0f; // El Boss es lento
        float moveStepX = cos(angleToPlayer) * speed * deltaTime;
        float moveStepY = sin(angleToPlayer) * speed * deltaTime;
        // Aquí deberías chequear colisiones con paredes (worldMap) antes de sumar

        // Definimos un margen de colisión (el "gordura" del enemigo)
        // 0.3f significa que el enemigo choca si se acerca a 30cm de la pared
        float buffer = 0.3f;
        // ---------------------------------------------------
        // EJE X: Intentar mover y verificar
        // ---------------------------------------------------
        // Calculamos la posición futura X con el buffer
        // Si nos movemos a la derecha (+), chequeamos un poco más a la derecha (+buffer)
        // Si nos movemos a la izquierda (-), chequeamos un poco más a la izquierda (-buffer)
        float checkX = x + moveStepX + (moveStepX > 0 ? buffer : -buffer);

        // Convertimos a coordenadas de mapa (enteros)
        int mapGridX = (int)checkX;
        int mapGridY = (int)y; // Mantenemos Y actual para chequear X

        // Si la celda es 0 (aire), permitimos el movimiento
        // Nota: Asegúrate de comprobar límites del array para no crashear (mapGridX >= 0 etc)
        if (worldMap[mapGridY][mapGridX] == 0) {
            x += moveStepX;
        }

        // ---------------------------------------------------
        // EJE Y: Intentar mover y verificar (Independiente de X)
        // ---------------------------------------------------
        float checkY = y + moveStepY + (moveStepY > 0 ? buffer : -buffer);

        mapGridX = (int)x; // Usamos la X (posiblemente nueva)
        int mapGridY_New = (int)checkY;

        if (worldMap[mapGridY_New][mapGridX] == 0) {
            y += moveStepY;
        }
    }
    else if (canSeePlayer){
        // --- ESTAMOS EN RANGO DE ATAQUE ---

        if (attackTimer <= 0) {
            // ¡ATACAR!
            attackTimer = timeBetweenAttacks; // Reset cooldown

            // Comportamiento según tipo
            if (type == TYPE_MELEE) {
                // Ataque cuerpo a cuerpo: Daño directo
                playerHP -= damage;
                std::cout << "Mordida! Salud Jugador: " << playerHP << std::endl;
                // Opcional: Reproducir sonido de mordida
            }
            else if (type == TYPE_RANGED || type == TYPE_BOSS) {
                // Ataque a distancia: Crear proyectil HOSTIL
                // Nace en la posición del enemigo y va hacia el jugador
                Projectile* p = new Projectile(x, y, angleToPlayer, bulletTex, true);
                projectiles.push_back(p);
                std::cout << "Enemigo dispara!" << std::endl;
            }
        }
    }

    // Actualizar animación base (idle/caminar)
    update(deltaTime);

}

bool Enemy::CheckLineOfSight(float x1, float y1, float x2, float y2, const int worldMap[30][30]) {
    // 1. En qué casilla del mapa están el inicio y el fin
    int mapX = (int)x1;
    int mapY = (int)y1;
    int targetX = (int)x2;
    int targetY = (int)y2;

    // Si están en la misma casilla, obviamente se ven
    if (mapX == targetX && mapY == targetY) return true;

    // 2. Calcular dirección y pasos (Matemática DDA estándar)
    float rayDirX = x2 - x1;
    float rayDirY = y2 - y1;

    // Delta Dist: Distancia que el rayo debe viajar para cruzar una unidad X o Y
    float deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1.0f / rayDirY);

    // Variables de paso
    int stepX, stepY;
    float sideDistX, sideDistY;

    // Calcular step y sideDist inicial
    if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (x1 - mapX) * deltaDistX;
    }
    else {
        stepX = 1;
        sideDistX = (mapX + 1.0f - x1) * deltaDistX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (y1 - mapY) * deltaDistY;
    }
    else {
        stepY = 1;
        sideDistY = (mapY + 1.0f - y1) * deltaDistY;
    }

    // 3. EL BUCLE DDA (Caminar por la cuadrícula)
    // Limitamos los pasos para evitar bucles infinitos (ej: max 20 cuadros de visión)
    int maxSteps = 20;

    for (int i = 0; i < maxSteps; i++) {
        // Saltar al siguiente cuadrado
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
        }
        else {
            sideDistY += deltaDistY;
            mapY += stepY;
        }

        // A. ¿Chocamos con una pared?
        // Asegúrate de revisar los límites del array [0-29]
        if (mapX < 0 || mapX >= 30 || mapY < 0 || mapY >= 30) return false;

        if (worldMap[mapY][mapX] > 0) {
            return false; // Bloqueado por pared
        }

        // B. ¿Llegamos al jugador?
        if (mapX == targetX && mapY == targetY) {
            return true; // ¡Te veo!
        }
    }

    return false; // Demasiado lejos
}