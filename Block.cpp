#include "Block.h"

namespace Arcanoid {

    Block::Block(float x, float y, sf::Color color) : isActive(true), color(color) {
        size = sf::Vector2f(60.0f, 25.0f);
        position = sf::Vector2f(x, y);

        sf::Image image;
        image.create(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y), color);
        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setPosition(position);
    }

    void Block::update(float dt) {
    }

    void Block::draw(sf::RenderWindow& window) const {
        if (isActive) {
            window.draw(sprite);
        }
    }

}