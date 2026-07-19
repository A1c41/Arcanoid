#include "GlassBlock.h"

namespace Arcanoid {

    GlassBlock::GlassBlock(float x, float y)
        : Block(x, y, sf::Color(200, 230, 255, 180)), isActive(true) {
        size = sf::Vector2f(60.0f, 25.0f);
        position = sf::Vector2f(x, y);

        sf::Image image;
        image.create(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y),
            sf::Color(200, 230, 255, 180));

        for (int px = 0; px < static_cast<int>(size.x); ++px) {
            for (int py = 0; py < static_cast<int>(size.y); ++py) {
                if (px == 0 || px == static_cast<int>(size.x) - 1 ||
                    py == 0 || py == static_cast<int>(size.y) - 1) {
                    image.setPixel(px, py, sf::Color(150, 200, 255, 200));
                }
                if ((px % 10 < 3 && py % 10 < 3) || (px % 10 > 7 && py % 10 > 7)) {
                    sf::Color pixel = image.getPixel(px, py);
                    pixel.r = std::min(255, pixel.r + 30);
                    pixel.g = std::min(255, pixel.g + 30);
                    pixel.b = std::min(255, pixel.b + 30);
                    image.setPixel(px, py, pixel);
                }
            }
        }

        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    void GlassBlock::draw(sf::RenderWindow& window) const {
        if (isActive) {
            window.draw(sprite);
        }
    }

}