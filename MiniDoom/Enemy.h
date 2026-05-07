#pragma once
#include "Sprite.h"
#include <vector>
#include "Game.h"

enum EnemyType {
    TYPE_MELEE,  // Zombie
    TYPE_RANGED, // Soldado
    TYPE_BOSS    // Cyberdemon
};
struct Projectile;
class Enemy : public Sprite {
public:
    EnemyType type;
    float attackTimer;
    float timeBetweenAttacks;
	float aggroRange;
    float attackRange;
    int damage;
	int mapWidth;
	int mapHeight;
	bool hasSeenThePlayer = false;
	bool canSeePlayer = false;
    float lastKnownX = 0.0f;
    float lastKnownY = 0.0f;
    bool hasDealtDamage = false;
    const float MELEE_RANGE = 1.5f; // A un metro y medio de distancia (cuerpo a cuerpo)
    const float RANGED_RANGE = 8.0f; // A 8 metros de distancia (disparo)
    std::vector<SDL_Surface*> animAttack; // Aquí guardarás las imágenes del ataque

    Enemy(float x, float y, SDL_Surface* tex, EnemyType t) : Sprite(x, y, tex, 1.0f) {
        type = t;
        attackTimer = 0.0f;
        defaultSurf = tex;
        // Configurar stats seg�n el tipo
        switch (type) {
        case TYPE_MELEE:
            hp = 50;
            scale = 1.0f;
            attackRange = 1.2f; // Tiene que estar cerca
			aggroRange = 10.0f;
            damage = 10;
            timeBetweenAttacks = 1.0f; // Pega cada 1 segundo
            break;

        case TYPE_RANGED:
            hp = 30; // Menos vida
            scale = 1.0f;
            attackRange = 7.0f; // Dispara de lejos
			aggroRange = 10.0f;
            damage = 15;
            timeBetweenAttacks = 2.0f;
            break;

        case TYPE_BOSS:
            hp = 500;
            scale = 2.5f; // GIGANTE
			aggroRange = 15.0f;
            attackRange = 10.0f;
            damage = 40;
            timeBetweenAttacks = 1.5f;
            break;
        }
    }

    void updateAI(float deltaTime, float playerX, float playerY, int& playerHP, std::vector<Projectile*>& projectiles, SDL_Surface* bulletTex, const std::vector<std::vector<int>>& worldMap);

    bool CheckLineOfSight(float x1, float y1, float x2, float y2, const std::vector<std::vector<int>>& worldMap);

    void addAttackFrame(SDL_Surface* surf);
};