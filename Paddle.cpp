#include "Paddle.h"

namespace Arcanoid {

    Paddle::Paddle() : speed(350.0f), moveLeft(false), moveRight(false), baseWidth(120.0f), baseHeight(22.0f) {
        size = sf::Vector2f(baseWidth, baseHeight);
        position = sf::Vector2f(WINDOW_WIDTH / 2.0f - size.x / 2, WINDOW_HEIGHT - 50.0f);

        shape.setSize(size);
        shape.setFillColor(sf::Color::White);
        shape.setOutlineColor(sf::Color(200, 200, 200));
        shape.setOutlineThickness(1.0f);
        shape.setPosition(position);
    }

    void Paddle::setSize(const sf::Vector2f& newSize) {
        if (newSize.x <= 0 || newSize.y <= 0) return;

        size = newSize;

        if (position.x + size.x > WINDOW_WIDTH) {
            position.x = WINDOW_WIDTH - size.x;
        }
        if (position.x < 0) {
            position.x = 0;
        }

        shape.setSize(size);
        shape.setPosition(position);
    }

    void Paddle::update(float dt) {
        if (moveLeft) {
            position.x -= speed * dt;
            if (position.x < 0) position.x = 0;
        }
        if (moveRight) {
            position.x += speed * dt;
            if (position.x + size.x > WINDOW_WIDTH) position.x = WINDOW_WIDTH - size.x;
        }
        shape.setPosition(position);
    }

    void Paddle::draw(sf::RenderWindow& window) const {
        window.draw(shape);
    }

    void Paddle::handleInput(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A) {
                moveLeft = true;
                moveRight = false;
            }
            if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) {
                moveRight = true;
                moveLeft = false;
            }
        }
        else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A) {
                moveLeft = false;
            }
            if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) {
                moveRight = false;
            }
        }
    }

    sf::FloatRect Paddle::getBounds() const {
        return shape.getGlobalBounds();
    }

}