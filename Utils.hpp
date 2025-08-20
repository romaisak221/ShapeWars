#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Shape.hpp"

float frand(float a, float b);
int irand(int a, int b);

sf::Text makeText(const sf::Font& font, unsigned size, const sf::String& s, float x, float y);

void spawnMiniGoods(std::vector<Shape>& shapes, sf::Vector2f pos, int count=4);
