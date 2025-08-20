#pragma once
#include <SFML/Graphics.hpp>
#include "Shape.hpp"
#include "PowerUp.hpp"

enum GameMode { MODE_NORMAL, MODE_TIMED, MODE_ENDLESS, MODE_HARDCORE };

class Game {
public:
    Game(sf::RenderWindow& window, sf::Font& font, GameMode mode);
    void run();

private:
    sf::RenderWindow& window;
    sf::Font& font;
    GameMode mode;

    int score, lives;
    float elapsed, spawnTimer, powerupTimer;
    std::vector<Shape> shapes;
    std::vector<PowerUp> powerups;

    void spawnShape();
    void spawnPowerUp();

    // 🔹 Add these:
    void showGameOver(const std::string& msg, int finalScore);
    std::string gameModeName(GameMode m);
};
