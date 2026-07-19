#include "GameState.h"
#include <cstdlib>
#include <ctime>

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    Arcanoid::GameState game;
    sf::Clock clock;

    while (game.isRunning()) {
        float dt = clock.restart().asSeconds();
        sf::Event event;
        while (game.getWindow().pollEvent(event)) {
            game.handleInput(event);
        }
        game.update(dt);
        game.render();
    }
    return 0;
}