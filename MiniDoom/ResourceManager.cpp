#include "ResourceManager.h"

SDL_Surface* ResourceManager::GetTexture(std::string path) {
    // 1. Buscar si ya la tenemos cargada
    auto it = textures.find(path);

    if (it != textures.end()) {
        // ¡Encontrada! Devolvemos el puntero existente
        return it->second;
    }

    // 2. No existe, hay que cargarla del disco
    std::cout << "Cargando textura nueva: " << path << std::endl;
    SDL_Surface* surf = Utils::LoadTexture(path.c_str());

    if (surf) {
        // Guardarla en el mapa para la próxima vez
        textures[path] = surf;
    }
    else {
        std::cout << "ERROR: No se pudo cargar " << path << std::endl;
    }

    return surf;
}

void ResourceManager::Clear() {
    // Recorrer todo el mapa y liberar memoria
    for (auto const& [name, surface] : textures) {
        if (surface) {
            SDL_FreeSurface(surface);
        }
    }
    textures.clear();
    std::cout << "Recursos liberados." << std::endl;
}