#include "Utils.hpp"
#include <random>
#include <ctime>
#include <cmath>

static std::mt19937 rng((unsigned)std::time(nullptr));

float frand(float a, float b) {
    std::uniform_real_distribution<float> d(a,b);
    return d(rng);
}

int irand(int a, int b) {
    std::uniform_int_distribution<int> d(a,b);
    return d(rng);
}

sf::Text makeText(const sf::Font& font, unsigned size, const sf::String& s, float x, float y) {
    sf::Text t;
    t.setFont(font);
    t.setCharacterSize(size);
    t.setString(s);
    t.setPosition(x,y);
    t.setFillColor(sf::Color::White);
    return t;
}

void spawnMiniGoods(std::vector<Shape>& shapes, sf::Vector2f pos, int count) {
    for (int i=0;i<count;i++) {
        float r = frand(5,10);
        Shape s(pos, {0,0}, SK_GOOD, r*2.f);
        float ang = frand(0,2*3.14159f);
        float sp = frand(80,180);
        s.vel = {std::cos(ang)*sp, std::sin(ang)*sp};
        shapes.push_back(s);
    }
}
