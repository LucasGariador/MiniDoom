#include "Game.h"

int main(int argc, char* argv[]) {
    Game game;

    // 1. Inicializar
    if (!game.init("MiniDoom", 800, 600)) {
        return 1;
    }

    // 2. Bucle
    while (game.running()) {
        game.handleEvents();
        game.update();
        game.render();
    }

    // 3. Limpiar
    game.clean();

    return 0;
}