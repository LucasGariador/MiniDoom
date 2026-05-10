#include "Game.h"

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init("MiniDoom", 800, 600)) {
        return 1;
    }

    while (game.running()) {
        if(game.currentState == Game::GameState::MAINMENU) {
			game.handleEvents();
			game.render();
		}
        if (game.currentState == Game::GameState::PLAYING) {
            game.handleEvents();
            game.update();
            game.render();
        }
    }

    game.clean();

    return 0;
}