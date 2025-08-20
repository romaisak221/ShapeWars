#include "Shape.hpp"
#include <cmath>

Shape::Shape() : alive(true), kind(SK_GOOD) {}

Shape::Shape(sf::Vector2f pos, sf::Vector2f vel, ShapeKind kind, float size)
    : vel(vel), kind(kind), alive(true) {
    circle.setRadius(size / 2.f);
    circle.setOrigin(size / 2.f, size / 2.f);
    circle.setPosition(pos);

    if (kind == SK_GOOD) circle.setFillColor(sf::Color(100,220,100));
    else if (kind == SK_BAD) circle.setFillColor(sf::Color(220,100,100));
    else circle.setFillColor(sf::Color(220,200,80));
}

void Shape::update(float dt, bool frozen) {
    if (!alive) return;
    if (!frozen) circle.move(vel * dt);
}

bool Shape::contains(const sf::Vector2f& point) const {
    sf::Vector2f pos = circle.getPosition();
    float dx = point.x - pos.x, dy = point.y - pos.y;
    return (dx*dx + dy*dy) <= (circle.getRadius()*circle.getRadius());
}
