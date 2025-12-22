#pragma once
#include "Sprite.h"
#include <vector> // Necesario para std::vector
#include "Game.h"

enum EnemyType {
    TYPE_MELEE,  // Zombie básico
    TYPE_RANGED, // Soldado con arma
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

    // Animación de ataque (Opcional, si tienes los sprites)
    // std::vector<SDL_Surface*> animAttack; 

    Enemy(float x, float y, SDL_Surface* tex, EnemyType t) : Sprite(x, y, tex, 1.0f) {
        type = t;
        attackTimer = 0.0f;

        // Configurar stats según el tipo
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

    // Necesitaremos acceso a la lista de proyectiles para disparar
    void updateAI(float deltaTime, float playerX, float playerY, int& playerHP, std::vector<Projectile*>& projectiles, SDL_Surface* bulletTex, const int worldMap[30][30]);

    bool CheckLineOfSight(float x1, float y1, float x2, float y2, const int worldMap[30][30]);
};