// main.cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>

const unsigned WINDOW_W = 900;
const unsigned WINDOW_H = 600;
const char* FONT_PATH = "assets/OpenSans-Regular.ttf";
const float PI = 3.14159265f;

static std::mt19937 rng((unsigned)std::time(nullptr));
static float frand(float a, float b) {
    std::uniform_real_distribution<float> d(a, b); return d(rng);
}
static int irand(int a, int b) {
    std::uniform_int_distribution<int> d(a, b); return d(rng);
}

enum GameMode { MODE_NORMAL, MODE_TIMED, MODE_ENDLESS, MODE_HARDCORE };

// ---------- Forward declarations ----------
class Game;
class Player;
class Entity;
class ShapeEntity;
class PowerUp;

// ---------- Utility helpers ----------
sf::Text makeText(const sf::Font& font, unsigned size, const sf::String& s, float x, float y) {
    sf::Text t; t.setFont(font); t.setCharacterSize(size); t.setString(s); t.setPosition(x, y); t.setFillColor(sf::Color::White);
    return t;
}
sf::CircleShape makeCircleShape(float radius, sf::Color fill) {
    sf::CircleShape c(radius);
    c.setOrigin(radius, radius);
    c.setFillColor(fill);
    return c;
}
bool contains(const sf::CircleShape& c, const sf::Vector2f& point) {
    sf::Vector2f pos = c.getPosition();
    float dx = point.x - pos.x, dy = point.y - pos.y;
    return (dx*dx + dy*dy) <= (c.getRadius()*c.getRadius());
}

// ---------- Player (score, lives, power timers) ----------
class Player {
    int score_ = 0;
    int lives_ = 3;
    float freezeTimer_ = 0.f;
    float doubleTimer_ = 0.f;
public:
    Player() = default;
    void setMode(GameMode m) {
        if (m == MODE_HARDCORE) lives_ = 1;
        else lives_ = 3;
    }
    int score() const { return score_; }
    int lives() const { return lives_; }
    void addScore(int v) { score_ += v; }
    void addLives(int v) { lives_ += v; }
    void loseLife(int v=1) { lives_ -= v; }
    void setFreeze(float t) { freezeTimer_ = std::max(freezeTimer_, t); }
    void setDouble(float t) { doubleTimer_ = std::max(doubleTimer_, t); }
    float freezeRemaining() const { return freezeTimer_; }
    float doubleRemaining() const { return doubleTimer_; }
    void updateTimers(float dt) {
        if (freezeTimer_ > 0.f) freezeTimer_ = std::max(0.f, freezeTimer_ - dt);
        if (doubleTimer_ > 0.f) doubleTimer_ = std::max(0.f, doubleTimer_ - dt);
    }
    bool isFrozen() const { return freezeTimer_ > 0.f; }
    bool doubleActive() const { return doubleTimer_ > 0.f; }
};

// ---------- Entity base ----------
class Entity {
protected:
    sf::CircleShape shape_;
    bool alive_ = true;
public:
    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& w) { if (alive_) w.draw(shape_); }
    virtual bool containsPoint(const sf::Vector2f& p) const { return contains(shape_, p); }
    bool alive() const { return alive_; }
    void kill() { alive_ = false; }
    const sf::CircleShape& shape() const { return shape_; }
    sf::CircleShape& shape() { return shape_; }
};

// ---------- ShapeEntity and concrete shapes ----------
class ShapeEntity : public Entity {
protected:
    sf::Vector2f vel_;
public:
    virtual void onClick(Player& player, std::vector<std::unique_ptr<ShapeEntity>>& shapes, sf::Sound* clickSound=nullptr, sf::Sound* badSound=nullptr) = 0;
    virtual void update(float dt) override {
        shape_.move(vel_ * dt);
    }
    void setVelocity(const sf::Vector2f& v) { vel_ = v; }
};

class GoodShape : public ShapeEntity {
public:
    GoodShape(float radius, sf::Vector2f pos, sf::Vector2f vel) {
        shape_ = makeCircleShape(radius, sf::Color(100,220,100));
        shape_.setPosition(pos);
        vel_ = vel;
        alive_ = true;
    }
    virtual void onClick(Player& player, std::vector<std::unique_ptr<ShapeEntity>>& /*shapes*/, sf::Sound* clickSound=nullptr, sf::Sound* /*badSound*/=nullptr) override {
        int pts = 10;
        if (player.doubleActive()) pts *= 2;
        player.addScore(pts);
        if (clickSound) clickSound->play();
        kill();
    }
};

class BadShape : public ShapeEntity {
public:
    BadShape(float radius, sf::Vector2f pos, sf::Vector2f vel) {
        shape_ = makeCircleShape(radius, sf::Color(220,100,100));
        shape_.setPosition(pos);
        vel_ = vel;
        alive_ = true;
    }
    virtual void onClick(Player& player, std::vector<std::unique_ptr<ShapeEntity>>& /*shapes*/, sf::Sound* /*clickSound*/=nullptr, sf::Sound* badSound=nullptr) override {
        player.addScore(-6);
        player.loseLife(1);
        if (badSound) badSound->play();
        kill();
    }
};

class ExploderShape : public ShapeEntity {
public:
    ExploderShape(float radius, sf::Vector2f pos, sf::Vector2f vel) {
        shape_ = makeCircleShape(radius, sf::Color(220,200,80));
        shape_.setPosition(pos);
        vel_ = vel;
        alive_ = true;
    }
    virtual void onClick(Player& player, std::vector<std::unique_ptr<ShapeEntity>>& shapes, sf::Sound* clickSound=nullptr, sf::Sound* /*badSound*/=nullptr) override {
        player.addScore(8);
        if (clickSound) clickSound->play();
        // spawn mini goods
        sf::Vector2f pos = shape_.getPosition();
        for (int i = 0; i < 4; ++i) {
            float r = frand(5.f, 10.f);
            float ang = frand(0.f, 2.f*PI);
            float sp = frand(80.f, 180.f);
            sf::Vector2f vel = { std::cos(ang)*sp, std::sin(ang)*sp };
            shapes.push_back(std::make_unique<GoodShape>(r, pos, vel));
        }
        kill();
    }
};

// ---------- PowerUps ----------
enum PowerUpType { PU_FREEZE, PU_DOUBLE, PU_EXTRA_LIFE };

class PowerUp : public Entity {
protected:
    PowerUpType type_;
    float duration_;
public:
    PowerUp(PowerUpType t, float duration, sf::Vector2f pos) : type_(t), duration_(duration) {
        shape_ = makeCircleShape(12.f, sf::Color::Blue);
        if (t == PU_DOUBLE) shape_.setFillColor(sf::Color(200,180,60));
        if (t == PU_EXTRA_LIFE) shape_.setFillColor(sf::Color(200,100,220));
        shape_.setPosition(pos);
        alive_ = true;
    }
    virtual void onCollect(Player& player, sf::Sound* puSound=nullptr) {
        if (type_ == PU_FREEZE) {
            player.setFreeze(duration_);
        } else if (type_ == PU_DOUBLE) {
            player.setDouble(duration_);
        } else if (type_ == PU_EXTRA_LIFE) {
            player.addLives(1);
        }
        if (puSound) puSound->play();
        kill();
    }
    virtual void update(float /*dt*/) override {
        // give a pulsing effect when drawn (handled in draw by scale)
    }
    PowerUpType type() const { return type_; }
};

// ---------- Spawner (handles spawn timing) ----------
class Spawner {
    float spawnTimer_ = 0.f;
    float spawnInterval_ = 0.8f;
    float powerupTimer_ = 0.f;
    float powerupInterval_ = 12.f;
    float elapsed_ = 0.f;
public:
    void reset() { spawnTimer_ = powerupTimer_ = elapsed_ = 0.f; spawnInterval_ = 0.8f; }
    void updateElapsed(float dt) { elapsed_ += dt; }
    float elapsed() const { return elapsed_; }

    // adjust spawn interval based on mode/time
    void recomputeInterval(GameMode mode) {
        spawnInterval_ = std::max(0.12f, 0.8f - elapsed_ * 0.005f);
        if (mode == MODE_HARDCORE) spawnInterval_ *= 0.5f;
        if (mode == MODE_ENDLESS) spawnInterval_ *= 0.9f;
    }

    bool shouldSpawnShape(float dt) {
        spawnTimer_ += dt;
        if (spawnTimer_ >= spawnInterval_) {
            spawnTimer_ = 0.f;
            return true;
        }
        return false;
    }
    bool shouldSpawnPowerUp(float dt) {
        powerupTimer_ += dt;
        if (powerupTimer_ >= powerupInterval_) {
            powerupTimer_ = 0.f;
            return true;
        }
        return false;
    }

    // factory to create shapes (random position around edges -> towards center)
    std::unique_ptr<ShapeEntity> createRandomShape(GameMode mode) {
        float w = (float)WINDOW_W, h = (float)WINDOW_H;
        int edge = irand(0, 3);
        sf::Vector2f pos;
        if (edge == 0) pos = {frand(20, w-20), -20.f};
        else if (edge == 1) pos = {frand(20, w-20), h + 20.f};
        else if (edge == 2) pos = {-20.f, frand(20, h-20)};
        else pos = {w + 20.f, frand(20, h-20)};

        sf::Vector2f center{w/2.f, h/2.f};
        sf::Vector2f dir = center - pos;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len != 0) dir /= len;
        else dir = {0.f, 1.f};

        float speedBase = frand(60.f, 140.f) + elapsed_ * 2.f;
        if (mode == MODE_HARDCORE) speedBase *= 1.5f;
        if (mode == MODE_ENDLESS) speedBase *= 1.2f;
        sf::Vector2f vel = dir * speedBase;

        int t = irand(0, 99);
        float size = frand(16.f, 34.f);
        if (t < 60) {
            return std::make_unique<GoodShape>(size/2.f, pos, vel);
        } else if (t < 85) {
            return std::make_unique<BadShape>(size/2.f, pos, vel);
        } else {
            return std::make_unique<ExploderShape>(size/2.f, pos, vel);
        }
    }

    std::unique_ptr<PowerUp> createRandomPowerUp() {
        float x = frand(50.f, WINDOW_W-50.f);
        float y = frand(50.f, WINDOW_H-50.f);
        int t = irand(0, 2);
        PowerUpType type = (t == 0 ? PU_FREEZE : (t == 1 ? PU_DOUBLE : PU_EXTRA_LIFE));
        float duration = (type == PU_EXTRA_LIFE) ? 0.f : (type == PU_FREEZE ? 3.f : 6.f);
        return std::make_unique<PowerUp>(type, duration, sf::Vector2f{x,y});
    }
};

// ---------- Menu ----------
class Menu {
public:
    GameMode run(sf::RenderWindow& window, sf::Font& font) {
        int sel = 0;
        std::vector<std::string> items = {"Start - Normal", "Start - Timed (60s)", "Start - Endless", "Start - Hardcore", "Exit"};
        sf::Text title = makeText(font, 48, "Shape Wars", 260.f, 80.f);
        sf::Text hint = makeText(font, 18, "Use Up/Down and Enter to select (or click)", 180.f, 520.f);

        while (window.isOpen()) {
            sf::Event e;
            while (window.pollEvent(e)) {
                if (e.type == sf::Event::Closed) window.close();
                if (e.type == sf::Event::KeyPressed) {
                    if (e.key.code == sf::Keyboard::Down) sel = (sel+1) % items.size();
                    if (e.key.code == sf::Keyboard::Up) sel = (sel-1 + items.size()) % items.size();
                    if (e.key.code == sf::Keyboard::Enter) {
                        if (sel == 0) return MODE_NORMAL;
                        if (sel == 1) return MODE_TIMED;
                        if (sel == 2) return MODE_ENDLESS;
                        if (sel == 3) return MODE_HARDCORE;
                        if (sel == 4) { window.close(); return MODE_NORMAL; }
                    }
                }
                if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f m((float)e.mouseButton.x, (float)e.mouseButton.y);
                    for (size_t i=0;i<items.size();++i) {
                        sf::Text t = makeText(font, 28, items[i], 300.f, 180.f + (float)i*50.f);
                        if (t.getGlobalBounds().contains(m)) {
                            if (i==0) return MODE_NORMAL;
                            if (i==1) return MODE_TIMED;
                            if (i==2) return MODE_ENDLESS;
                            if (i==3) return MODE_HARDCORE;
                            if (i==4) { window.close(); return MODE_NORMAL; }
                        }
                    }
                }
            }

            window.clear(sf::Color(20,20,30));
            window.draw(title);
            for (size_t i=0;i<items.size();++i) {
                sf::Text t = makeText(font, 28, (i==sel? "> ":"  ") + items[i], 300.f, 180.f + (float)i*50.f);
                if (i==sel) t.setFillColor(sf::Color::Yellow);
                window.draw(t);
            }
            window.draw(hint);
            window.display();
        }
        return MODE_NORMAL;
    }
};

// ---------- Game (manages everything) ----------
class Game {
    sf::RenderWindow window_;
    sf::Font font_;
    Menu menu_;
    Player player_;
    Spawner spawner_;
    GameMode mode_ = MODE_NORMAL;

    std::vector<std::unique_ptr<ShapeEntity>> shapes_;
    std::vector<std::unique_ptr<PowerUp>> powerups_;

    // audio
    sf::SoundBuffer clickBuf_, badBuf_, puBuf_;
    sf::Sound sClick_, sBad_, sPU_;
    bool hasAudio_ = false;

    // HUD & misc
    float timedCountdown_ = 0.f;
public:
    Game() : window_(sf::VideoMode(WINDOW_W, WINDOW_H), "Shape Wars - OOP") {
        window_.setFramerateLimit(60);
        if (!font_.loadFromFile(FONT_PATH)) {
            std::cerr << "Warning: failed to load font at " << FONT_PATH << "\n"
                      << "HUD text may not display. Add the font file in assets/ to fix.\n";
        }
        if (clickBuf_.loadFromFile("assets/click.wav")) { hasAudio_ = true; sClick_.setBuffer(clickBuf_); }
        if (badBuf_.loadFromFile("assets/error.wav")) { sBad_.setBuffer(badBuf_); }
        if (puBuf_.loadFromFile("assets/powerup.wav")) { sPU_.setBuffer(puBuf_); }
    }

    void run() {
        while (window_.isOpen()) {
            mode_ = menu_.run(window_, font_);
            if (!window_.isOpen()) break;
            startGameMode(mode_);
            runGame();
            // return to menu automatically and loop
        }
    }

private:
    void startGameMode(GameMode m) {
        mode_ = m;
        player_ = Player();
        player_.setMode(m);
        spawner_.reset();
        shapes_.clear();
        powerups_.clear();
        if (mode_ == MODE_TIMED) timedCountdown_ = 60.f;
        else timedCountdown_ = 0.f;
        // pre-seed some shapes
        for (int i=0;i<6;i++) {
            shapes_.push_back(spawner_.createRandomShape(mode_));
        }
    }

    void runGame() {
        sf::Clock clock;
        float bgTimer = 0.f;
        sf::Color bgColor = sf::Color(28,28,40);

        sf::Text hud = makeText(font_, 20, "", 8.f, 8.f);
        sf::Text modeText = makeText(font_, 18, "", 8.f, 36.f);
        sf::Text instructions = makeText(font_, 16, "Left click shapes. ESC to quit to menu.", 8.f, WINDOW_H - 28.f);

        while (window_.isOpen()) {
            float dt = clock.restart().asSeconds();
            spawner_.updateElapsed(dt);
            player_.updateTimers(dt);
            if (mode_ == MODE_TIMED) timedCountdown_ -= dt;
            if (timedCountdown_ < 0 && mode_ == MODE_TIMED) timedCountdown_ = 0.f;

            spawner_.recomputeInterval(mode_);

            // events
            sf::Event e;
            while (window_.pollEvent(e)) {
                if (e.type == sf::Event::Closed) { window_.close(); return; }
                if (e.type == sf::Event::KeyPressed) {
                    if (e.key.code == sf::Keyboard::Escape) return;
                }
                if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
                    // check powerups first
                    bool processed = false;
                    for (auto &pu : powerups_) {
                        if (!pu->alive()) continue;
                        if (pu->containsPoint(mp)) {
                            pu->onCollect(player_, hasAudio_ ? &sPU_ : nullptr);
                            processed = true;
                            break;
                        }
                    }
                    if (processed) continue;
                    // check shapes from topmost
                    for (int i = (int)shapes_.size()-1; i >= 0; --i) {
                        if (!shapes_[i]->alive()) continue;
                        if (shapes_[i]->containsPoint(mp)) {
                            shapes_[i]->onClick(player_, shapes_, hasAudio_ ? &sClick_ : nullptr, hasAudio_ ? &sBad_ : nullptr);
                            break;
                        }
                    }
                }
            }

            // spawn shapes
            if (spawner_.shouldSpawnShape(dt)) {
                shapes_.push_back(spawner_.createRandomShape(mode_));
            }
            // spawn powerups occasionally
            if (spawner_.shouldSpawnPowerUp(dt)) {
                powerups_.push_back(spawner_.createRandomPowerUp());
            }

            // update shapes
            float moveFactor = player_.isFrozen() ? 0.f : 1.f;
            for (auto &s : shapes_) {
                if (s->alive()) {
                    // only move if not frozen
                    if (moveFactor > 0.f) s->update(dt);
                }
            }
            // cleanup shapes offscreen or dead
            for (auto it = shapes_.begin(); it != shapes_.end();) {
                if (!(*it)->alive()) { it = shapes_.erase(it); continue; }
                sf::Vector2f p = (*it)->shape().getPosition();
                if (p.x < -60.f || p.x > WINDOW_W + 60.f || p.y < -60.f || p.y > WINDOW_H + 60.f) it = shapes_.erase(it);
                else ++it;
            }

            // update powerups (pulsing handled on draw)
            for (auto it = powerups_.begin(); it != powerups_.end();) {
                if (!(*it)->alive()) it = powerups_.erase(it);
                else ++it;
            }

            // Check game over conditions
            if (mode_ == MODE_NORMAL || mode_ == MODE_HARDCORE) {
                if (player_.lives() <= 0) {
                    // show game over screen until ESC pressed
                    sf::RectangleShape box({(float)WINDOW_W, (float)WINDOW_H});
                    box.setFillColor(sf::Color(0,0,0,160));
                    sf::Text gameOver = makeText(font_, 48, "GAME OVER", 320.f, 220.f);
                    sf::Text final = makeText(font_, 28, ("Score: " + std::to_string(player_.score()) + "   Press ESC to go back"), 260.f, 300.f);
                    window_.clear(bgColor);
                    for (auto &s : shapes_) if (s->alive()) window_.draw(s->shape());
                    for (auto &p : powerups_) if (p->alive()) window_.draw(p->shape());
                    window_.draw(box);
                    window_.draw(gameOver);
                    window_.draw(final);
                    window_.display();

                    bool waiting = true;
                    while (waiting && window_.isOpen()) {
                        sf::Event ev;
                        while (window_.pollEvent(ev)) {
                            if (ev.type == sf::Event::Closed) { window_.close(); return; }
                            if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape) waiting = false;
                        }
                    }
                    return;
                }
            } else if (mode_ == MODE_TIMED) {
                if (timedCountdown_ <= 0.f) {
                    sf::RectangleShape box({(float)WINDOW_W, (float)WINDOW_H});
                    box.setFillColor(sf::Color(0,0,0,160));
                    std::ostringstream ss; ss << "TIME UP! Score: " << player_.score() << "   Press ESC to go back";
                    sf::Text final = makeText(font_, 28, ss.str(), 200.f, 300.f);
                    window_.clear(bgColor);
                    window_.draw(box);
                    window_.draw(final);
                    window_.display();
                    bool waiting = true;
                    while (waiting && window_.isOpen()) {
                        sf::Event ev;
                        while (window_.pollEvent(ev)) {
                            if (ev.type == sf::Event::Closed) { window_.close(); return; }
                            if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape) waiting = false;
                        }
                    }
                    return;
                }
            }

            // background color shift
            bgTimer += dt;
            bgColor.r = (sf::Uint8)(28 + 20 * std::sin(bgTimer * 0.25f));
            bgColor.g = (sf::Uint8)(28 + 12 * std::sin(bgTimer * 0.3f + 1.f));
            bgColor.b = (sf::Uint8)(40 + 16 * std::sin(bgTimer * 0.2f + 2.f));

            // HUD
            std::ostringstream hudss;
            hudss << "Score: " << player_.score() << "    Lives: " << player_.lives();
            if (mode_ == MODE_TIMED) hudss << "    Time: " << (int)std::ceil(timedCountdown_);
            if (player_.freezeRemaining() > 0.f) hudss << "    Freeze: " << std::fixed << std::setprecision(1) << player_.freezeRemaining();
            if (player_.doubleRemaining() > 0.f) hudss << "    2x: " << std::fixed << std::setprecision(1) << player_.doubleRemaining();
            hud.setString(hudss.str());

            // draw everything
            window_.clear(bgColor);
            for (auto &s : shapes_) {
                if (s->alive()) window_.draw(s->shape());
            }
            // draw powerups with subtle pulse
            for (auto &p : powerups_) {
                float sc = 1.f + 0.08f * std::sin(spawner_.elapsed() * 6.f + p->shape().getPosition().x);
                p->shape().setScale(sc, sc);
                window_.draw(p->shape());
            }

            window_.draw(hud);
            std::string modeName = (mode_==MODE_NORMAL?"Normal":mode_==MODE_TIMED?"Timed":mode_==MODE_ENDLESS?"Endless":"Hardcore");
            modeText.setString("Mode: " + modeName);
            window_.draw(modeText);
            window_.draw(instructions);

            window_.display();
        }
    }
};

// ---------- main ----------
int main() {
    Game game;
    game.run();
    return 0;
}
