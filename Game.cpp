#include "Game.hpp"
#include "Utils.hpp"
#include <SFML/Audio.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

Game::Game(sf::RenderWindow& win, sf::Font& f, GameMode m)
    : window(win), font(f), mode(m),
      score(0), lives(m == MODE_HARDCORE ? 1 : 3),
      elapsed(0.f), spawnTimer(0.f), powerupTimer(0.f) {}

void Game::run() {
    float timedCountdown = (mode == MODE_TIMED) ? 60.f : 0.f;

    // HUD
    sf::Text hud = makeText(font, 20, "", 8.f, 8.f);
    sf::Text modeText = makeText(font, 18, "", 8.f, 36.f);
    sf::Text instructions = makeText(font, 16, "Left click shapes. ESC to quit to menu.", 8.f, 570.f);

    sf::Clock clock;
    float freezeTimer = 0.f;
    float doublePointsTimer = 0.f;

    // Background
    float bgTimer = 0.f;
    sf::Color bgColor(28, 28, 40);

    // Sounds
    sf::SoundBuffer clickBuf, badBuf, puBuf;
    sf::Sound sClick, sBad, sPU;
    bool hasAudio = false;
    if (clickBuf.loadFromFile("assets/click.wav")) {
        hasAudio = true;
        sClick.setBuffer(clickBuf);
    }
    if (badBuf.loadFromFile("assets/error.wav")) sBad.setBuffer(badBuf);
    if (puBuf.loadFromFile("assets/powerup.wav")) sPU.setBuffer(puBuf);

    // Pre-spawn
    for (int i=0;i<6;i++) spawnShape();

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        elapsed += dt;
        if (mode == MODE_TIMED) timedCountdown -= dt;
        if (timedCountdown < 0 && mode == MODE_TIMED) timedCountdown = 0.f;

        // Update power-up timers
        if (freezeTimer > 0) freezeTimer -= dt;
        if (doublePointsTimer > 0) doublePointsTimer -= dt;

        // Event handling
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) { window.close(); return; }
            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) return;

            if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);

                // check powerups first
                for (auto &pu : powerups) {
                    if (pu.alive && pu.contains(mp)) {
                        if (pu.type == PU_FREEZE) freezeTimer = std::max(freezeTimer, 3.f);
                        else if (pu.type == PU_DOUBLE) doublePointsTimer = std::max(doublePointsTimer, 6.f);
                        else if (pu.type == PU_EXTRA_LIFE) lives++;
                        if (hasAudio) sPU.play();
                        pu.alive = false;
                        goto afterClick;
                    }
                }

                // check shapes
                for (int i = (int)shapes.size()-1; i >= 0; --i) {
                    if (!shapes[i].alive) continue;
                    if (shapes[i].contains(mp)) {
                        if (shapes[i].kind == SK_GOOD) {
                            int pts = (doublePointsTimer>0) ? 20 : 10;
                            score += pts;
                            if (hasAudio) sClick.play();
                        } else if (shapes[i].kind == SK_BAD) {
                            score -= 6; lives--;
                            if (hasAudio) sBad.play();
                        } else if (shapes[i].kind == SK_EXPLODER) {
                            score += 8;
                            spawnMiniGoods(shapes, shapes[i].circle.getPosition());
                            if (hasAudio) sClick.play();
                        }
                        shapes[i].alive = false;
                        break;
                    }
                }
            }
            afterClick: ;
        }

        // Spawn logic
        spawnTimer += dt;
        float spawnInterval = std::max(0.12f, 0.8f - elapsed * 0.005f);
        if (mode == MODE_HARDCORE) spawnInterval *= 0.5f;
        if (mode == MODE_ENDLESS) spawnInterval *= 0.9f;
        if (spawnTimer >= spawnInterval) {
            spawnTimer = 0.f;
            spawnShape();
        }

        powerupTimer += dt;
        if (powerupTimer >= 12.f) {
            powerupTimer = 0.f;
            spawnPowerUp();
        }

        // Update shapes
        for (auto &s : shapes) s.update(dt, freezeTimer>0);

        // Remove dead
        shapes.erase(std::remove_if(shapes.begin(), shapes.end(),
                    [](Shape& s){ return !s.alive; }), shapes.end());
        powerups.erase(std::remove_if(powerups.begin(), powerups.end(),
                    [](PowerUp& p){ return !p.alive; }), powerups.end());

        // Game over conditions
        if ((mode==MODE_NORMAL || mode==MODE_HARDCORE) && lives <= 0) {
            showGameOver("GAME OVER", score);
            return;
        }
        if (mode==MODE_TIMED && timedCountdown <= 0) {
            showGameOver("TIME UP!", score);
            return;
        }

        // Background shift
        bgTimer += dt;
        bgColor.r = (sf::Uint8)(28 + 20 * std::sin(bgTimer*0.25f));
        bgColor.g = (sf::Uint8)(28 + 12 * std::sin(bgTimer*0.3f+1.f));
        bgColor.b = (sf::Uint8)(40 + 16 * std::sin(bgTimer*0.2f+2.f));

        // HUD update
        std::ostringstream hudss;
        hudss << "Score: " << score << "   Lives: " << lives;
        if (mode==MODE_TIMED) hudss << "   Time: " << (int)std::ceil(timedCountdown);
        if (freezeTimer>0) hudss << "   Freeze: " << std::fixed << std::setprecision(1) << freezeTimer;
        if (doublePointsTimer>0) hudss << "   2x: " << std::fixed << std::setprecision(1) << doublePointsTimer;
        hud.setString(hudss.str());
        modeText.setString("Mode: " + gameModeName(mode));

        // Draw
        window.clear(bgColor);
        for (auto &s : shapes) window.draw(s.circle);
        for (auto &p : powerups) window.draw(p.icon);
        window.draw(hud);
        window.draw(modeText);
        window.draw(instructions);
        window.display();
    }
}

void Game::spawnShape() {
    float w = (float)window.getSize().x, h = (float)window.getSize().y;
    int edge = irand(0,3);
    sf::Vector2f pos;
    if (edge==0) pos = {frand(20,w-20), -20};
    else if (edge==1) pos = {frand(20,w-20), h+20};
    else if (edge==2) pos = {-20, frand(20,h-20)};
    else pos = {w+20, frand(20,h-20)};

    sf::Vector2f center(w/2.f, h/2.f);
    sf::Vector2f dir = center - pos;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    dir = (len!=0) ? dir/len : sf::Vector2f(0,1);

    float speed = frand(60.f,140.f) + elapsed*2.f;
    if (mode==MODE_HARDCORE) speed*=1.5f;
    if (mode==MODE_ENDLESS) speed*=1.2f;
    sf::Vector2f vel = dir * speed;

    int t = irand(0,99);
    ShapeKind kind = (t<60?SK_GOOD:(t<85?SK_BAD:SK_EXPLODER));
    float size = frand(16.f,34.f);

    shapes.emplace_back(pos, vel, kind, size);
}

void Game::spawnPowerUp() {
    sf::Vector2f pos(frand(50, window.getSize().x-50), frand(50, window.getSize().y-50));
    PowerUpType type = (PowerUpType)irand(0,2);
    powerups.emplace_back(pos, type);
}

void Game::showGameOver(const std::string& msg, int finalScore) {
    sf::RectangleShape box({(float)window.getSize().x, (float)window.getSize().y});
    box.setFillColor(sf::Color(0,0,0,160));
    sf::Text over = makeText(font, 48, msg, 280, 220);
    sf::Text scoreText = makeText(font, 28, "Score: " + std::to_string(finalScore) + "   Press ESC", 240, 300);

    bool waiting = true;
    while (waiting && window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type==sf::Event::Closed) { window.close(); return; }
            if (e.type==sf::Event::KeyPressed && e.key.code==sf::Keyboard::Escape) waiting = false;
        }
        window.clear();
        window.draw(box);
        window.draw(over);
        window.draw(scoreText);
        window.display();
    }
}

std::string Game::gameModeName(GameMode m) {
    switch(m) {
        case MODE_NORMAL: return "Normal";
        case MODE_TIMED: return "Timed";
        case MODE_ENDLESS: return "Endless";
        case MODE_HARDCORE: return "Hardcore";
    }
    return "Unknown";
}
