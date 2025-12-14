#pragma once
#include <SDL.h>
#include <map>
#include <string>
#include <iostream>
#include "Utils.h" // Asumo que aquí tienes tu LoadTexture

class ResourceManager {
public:
    // Singleton: Para poder acceder a él desde cualquier lado (Game, Sprite, etc.)
    static ResourceManager& Get() {
        static ResourceManager instance;
        return instance;
    }

    // Pide una textura. Si ya existe, te la da. Si no, la carga primero.
    SDL_Surface* GetTexture(std::string path);

    // Borra todo al cerrar el juego
    void Clear();

private:
    ResourceManager() {} // Constructor privado (nadie puede crear otro manager)

    // El "almacén"
    std::map<std::string, SDL_Surface*> textures;
};