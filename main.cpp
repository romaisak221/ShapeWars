#include <SFML/Graphics.hpp>
#include "Menu.hpp"
#include "Game.hpp"

const unsigned WINDOW_W = 900;
const unsigned WINDOW_H = 600;
const char* FONT_PATH = "assets/OpenSans-Regular.ttf";

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Shape Wars");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile(FONT_PATH)) {
        return -1;
    }

    while (window.isOpen()) {
        Menu menu(window, font);
        GameMode mode = menu.run();
        if (!window.isOpen()) break;

        Game game(window, font, mode);
        game.run();
    }
    return 0;
}
