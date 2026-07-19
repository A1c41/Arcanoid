#include "IndestructibleBlock.h"

namespace Arcanoid {

    IndestructibleBlock::IndestructibleBlock(float x, float y)
        : Block(x, y, sf::Color(80, 80, 80)), isActive(true) {
        size = sf::Vector2f(60.0f, 25.0f);
        position = sf::Vector2f(x, y);

        sf::Image image;
        image.create(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y),
            sf::Color(80, 80, 80));

        for (int px = 0; px < static_cast<int>(size.x); ++px) {
            for (int py = 0; py < static_cast<int>(size.y); ++py) {
                if (px == 0 || px == static_cast<int>(size.x) - 1 ||
                    py == 0 || py == static_cast<int>(size.y) - 1) {
                    image.setPixel(px, py, sf::Color(150, 150, 150));
                }
                if (px == static_cast<int>(size.x) / 2 || py == static_cast<int>(size.y) / 2) {
                    image.setPixel(px, py, sf::Color(120, 120, 120));
                }
                if ((px < 5 && py < 5) || (px > static_cast<int>(size.x) - 6 && py < 5) ||
                    (px < 5 && py > static_cast<int>(size.y) - 6) ||
                    (px > static_cast<int>(size.x) - 6 && py > static_cast<int>(size.y) - 6)) {
                    image.setPixel(px, py, sf::Color(180, 180, 180));
                }
            }
        }

        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    void IndestructibleBlock::draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

}