#pragma once
#include <SFML/Graphics.hpp>
#include "Game.hpp"

class Menu {
public:
    Menu(sf::RenderWindow& window, sf::Font& font);
    GameMode run();
private:
    sf::RenderWindow& window;
    sf::Font& font;
};
