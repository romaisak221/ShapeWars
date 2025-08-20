#pragma once
#include <SFML/Graphics.hpp>

enum PowerUpType { PU_FREEZE, PU_DOUBLE, PU_EXTRA_LIFE };

class PowerUp {
public:
    sf::CircleShape icon;
    PowerUpType type;
    float duration;
    bool alive;

    PowerUp();
    PowerUp(sf::Vector2f pos, PowerUpType type);

    bool contains(const sf::Vector2f& point) const;
};
