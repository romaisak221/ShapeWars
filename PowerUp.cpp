#include "PowerUp.hpp"
#include "Utils.hpp"

PowerUp::PowerUp() : alive(true), type(PU_FREEZE), duration(0) {}

PowerUp::PowerUp(sf::Vector2f pos, PowerUpType t) : alive(true), type(t) {
    icon.setRadius(12);
    icon.setOrigin(12,12);
    icon.setPosition(pos);
    duration = (t==PU_EXTRA_LIFE) ? 0.f : (t==PU_FREEZE ? 3.f : 6.f);
    if (t==PU_FREEZE) icon.setFillColor(sf::Color::Blue);
    if (t==PU_DOUBLE) icon.setFillColor(sf::Color(200,180,60));
    if (t==PU_EXTRA_LIFE) icon.setFillColor(sf::Color(200,100,220));
}

bool PowerUp::contains(const sf::Vector2f& point) const {
    sf::Vector2f pos = icon.getPosition();
    float dx = point.x-pos.x, dy=point.y-pos.y;
    return (dx*dx+dy*dy) <= (icon.getRadius()*icon.getRadius());
}
