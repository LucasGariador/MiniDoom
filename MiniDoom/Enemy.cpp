#include "Enemy.h"
#include "Projectile.h"

void Enemy::updateAI(float deltaTime, float playerX, float playerY, int& playerHP, std::vector<Projectile*>& projectiles, SDL_Surface* bulletTex, const int worldMap[7][7]) {
    
    if (state == STATE_DEAD || state == STATE_DYING) {
        update(deltaTime);
        return;
    }

    // 1. MEMORIA Y VISIÓN (Se actualiza SIEMPRE, sin importar el estado)
    float dx = playerX - x;
    float dy = playerY - y;
    float distSq = (dx * dx) + (dy * dy); 
    
    float activeRange = (type == TYPE_MELEE) ? MELEE_RANGE : RANGED_RANGE;
    float activeRangeSq = activeRange * activeRange;

    canSeePlayer = CheckLineOfSight(x, y, playerX, playerY, worldMap);
    if (canSeePlayer) {
        hasSeenThePlayer = true;
        lastKnownX = playerX; 
        lastKnownY = playerY;
    }

    if (attackTimer > 0) attackTimer -= deltaTime;

    // 2. MÁQUINA DE ESTADOS ESTRICTA
    switch (state) {
        
        case STATE_IDLE:
            // --- TRANSICIONES DESDE IDLE ---
            if (canSeePlayer && distSq <= activeRangeSq && attackTimer <= 0) {
                state = STATE_ATTACKING;
                animFrame = 0;
                animTimer = 0.0f;
                hasDealtDamage = false;
            } 
            else if (hasSeenThePlayer) {
                state = STATE_WALKING; // Cambia a caminar, pero no se mueve en este frame
            }
            break;

        case STATE_WALKING:
            // --- TRANSICIONES DESDE WALKING ---
            if (canSeePlayer && distSq <= activeRangeSq && attackTimer <= 0) {
                state = STATE_ATTACKING;
                animFrame = 0;
                animTimer = 0.0f;
                hasDealtDamage = false;
                break;
            }

            // --- COMPORTAMIENTO: PERSEGUIR ---
            {
                float targetDx = lastKnownX - x;
                float targetDy = lastKnownY - y;
                float distToTargetSq = (targetDx * targetDx) + (targetDy * targetDy);

                if (distToTargetSq < 0.1f) {
                    hasSeenThePlayer = false;
                    state = STATE_IDLE; // Llegó y no te vio, se queda quieto
                } else {
                    // Cálculo de movimiento exacto
                    float angleToTarget = atan2(targetDy, targetDx);
                    float speed = (type == TYPE_BOSS) ? 1.0f : 2.0f;
                    
                    float moveStepX = cos(angleToTarget) * speed * deltaTime;
                    float moveStepY = sin(angleToTarget) * speed * deltaTime;
                    float buffer = 0.15f; 

                    // Buffer X
                    float bufferX = 0.0f;
                    if (moveStepX > 0.001f) bufferX = buffer;
                    else if (moveStepX < -0.001f) bufferX = -buffer;
                    
                    if ((int)(x + moveStepX + bufferX) >= 0 && (int)(x + moveStepX + bufferX) < mapWidth && worldMap[(int)y][(int)(x + moveStepX + bufferX)] == 0) {
                        x += moveStepX;
                    }

                    // Buffer Y
                    float bufferY = 0.0f;
                    if (moveStepY > 0.001f) bufferY = buffer;
                    else if (moveStepY < -0.001f) bufferY = -buffer;

                    if ((int)(y + moveStepY + bufferY) >= 0 && (int)(y + moveStepY + bufferY) < mapHeight && worldMap[(int)(y + moveStepY + bufferY)][(int)x] == 0) {
                        y += moveStepY;
                    }
                }
            }
            break;

        case STATE_ATTACKING:
            // --- COMPORTAMIENTO: ATACAR ---
            // ¡Aquí adentro NO HAY CÓDIGO DE MOVIMIENTO!
            animTimer += deltaTime;
            if (animTimer >= animSpeed) {
                animTimer = 0.0f;
                animFrame++;
                
                if (animFrame == 4 && !hasDealtDamage) { 
                    hasDealtDamage = true; 
                    if (type == TYPE_MELEE && distSq <= activeRangeSq) {
                        playerHP -= damage;
                    } else if (type == TYPE_RANGED || type == TYPE_BOSS) {
                        float angleToPlayer = atan2(dy, dx);
                        projectiles.push_back(new Projectile(x, y, angleToPlayer, bulletTex, true));
                    }
                }
                
                if (animFrame >= animAttack.size()) {
                    state = STATE_IDLE; // Vuelve a evaluar en el próximo frame
                    attackTimer = timeBetweenAttacks;
                    currentSurf = defaultSurf;
                } else {
                    currentSurf = animAttack[animFrame]; 
                }
            }
            break;
    }

    // 3. ACTUALIZAR ANIMACIÓN BASE DE SPRITE
    update(deltaTime);
}

bool Enemy::CheckLineOfSight(float x1, float y1, float x2, float y2, const int worldMap[7][7]) {
    int mapX = (int)x1;
    int mapY = (int)y1;
    int targetX = (int)x2;
    int targetY = (int)y2;

    if (mapX == targetX && mapY == targetY) return true;

    // 2. Calcular direcci�n y pasos 
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

    int maxSteps = 100;

    for (int i = 0; i < maxSteps; i++) {
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
        }
        else {
            sideDistY += deltaDistY;
            mapY += stepY;
        }

        if (mapX < 0 || mapX >= 7 || mapY < 0 || mapY >= 7) return false;

        if (worldMap[mapY][mapX] > 0) {
            return false;
        }

        if (mapX == targetX && mapY == targetY) {
            return true;
        }
    }

    return false;
}

void Enemy::addAttackFrame(SDL_Surface* surf) {
    animAttack.push_back(surf);
}