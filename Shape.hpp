#pragma once
#include <SFML/Graphics.hpp>

enum ShapeKind { SK_GOOD, SK_BAD, SK_EXPLODER };

class Shape {
public:
    sf::CircleShape circle;
    sf::Vector2f vel;
    ShapeKind kind;
    bool alive;

    Shape();
    Shape(sf::Vector2f pos, sf::Vector2f vel, ShapeKind kind, float size);

    void update(float dt, bool frozen);
    bool contains(const sf::Vector2f& point) const;
};
