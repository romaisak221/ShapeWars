#include "Menu.hpp"
#include "Utils.hpp"

Menu::Menu(sf::RenderWindow& win, sf::Font& f) : window(win), font(f) {}

GameMode Menu::run() {
    int sel = 0;
    std::vector<std::string> items = {
        "Start - Normal", "Start - Timed (60s)", "Start - Endless", "Start - Hardcore", "Exit"
    };
    sf::Text title = makeText(font, 48, "Shape Wars", 260, 80);
    sf::Text hint  = makeText(font, 18, "Use Up/Down + Enter, or click", 200, 520);

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type==sf::Event::Closed) window.close();
            if (e.type==sf::Event::KeyPressed) {
                if (e.key.code==sf::Keyboard::Down) sel=(sel+1)%items.size();
                if (e.key.code==sf::Keyboard::Up) sel=(sel-1+items.size())%items.size();
                if (e.key.code==sf::Keyboard::Enter) {
                    if (sel==0) return MODE_NORMAL;
                    if (sel==1) return MODE_TIMED;
                    if (sel==2) return MODE_ENDLESS;
                    if (sel==3) return MODE_HARDCORE;
                    if (sel==4) { window.close(); return MODE_NORMAL; }
                }
            }
            if (e.type==sf::Event::MouseButtonPressed && e.mouseButton.button==sf::Mouse::Left) {
                sf::Vector2f m((float)e.mouseButton.x,(float)e.mouseButton.y);
                for (size_t i=0;i<items.size();i++) {
                    sf::Text t = makeText(font, 28, items[i], 300, 180+(float)i*50);
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
        for (size_t i=0;i<items.size();i++) {
            sf::Text t = makeText(font, 28, (i==sel?"> ":"  ")+items[i], 300, 180+(float)i*50);
            if (i==sel) t.setFillColor(sf::Color::Yellow);
            window.draw(t);
        }
        window.draw(hint);
        window.display();
    }
    return MODE_NORMAL;
}
