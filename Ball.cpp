#include "Ball.h"
#include <cmath>
#include <cstdlib>

namespace Arcanoid {

    Ball::Ball(float initialSpeed) : speed(initialSpeed) {
        size = sf::Vector2f(14.0f, 14.0f);
        position = sf::Vector2f(WINDOW_WIDTH / 2.0f - size.x / 2, WINDOW_HEIGHT / 2.0f - size.y / 2);

        sf::Image image;
        image.create(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y), sf::Color::White);
        texture.loadFromImage(image);
        sprite.setTexture(texture);
        sprite.setPosition(position);

        resetDirection();
    }

    void Ball::resetDirection() {
        float angle = (rand() % 60 - 30) * 3.14159f / 180.0f;
        velocity.x = (rand() % 2 == 0 ? 1 : -1) * speed * cos(angle);
        velocity.y = -speed * sin(std::abs(angle) + 0.3f);
        if (std::abs(velocity.y) < speed * 0.2f) velocity.y = -speed * 0.3f;
    }

    void Ball::update(float dt) {
        position += velocity * dt;
        sprite.setPosition(position);

        if (position.x < 0 || position.x + size.x > WINDOW_WIDTH) {
            velocity.x = -velocity.x;
        }
        if (position.y < 0) {
            velocity.y = -velocity.y;
        }
    }

    void Ball::draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }

    void Ball::bounceFromPaddle(const GameObject& paddle) {
        sf::FloatRect ballBounds = getBounds();
        sf::FloatRect paddleBounds = paddle.getBounds();

        if (ballBounds.intersects(paddleBounds)) {
            float relativeX = (position.x + size.x / 2 - paddle.getPosition().x - paddleBounds.width / 2) / (paddleBounds.width / 2);
            relativeX = std::max(-1.0f, std::min(1.0f, relativeX));

            float angle = relativeX * 60.0f * 3.14159f / 180.0f;
            float currentSpeed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

            velocity.x = currentSpeed * sin(angle);
            velocity.y = -currentSpeed * cos(angle);

            position.y = paddle.getPosition().y - size.y;
        }
    }

    bool Ball::isOutOfBounds() const {
        return position.y > WINDOW_HEIGHT;
    }

    void Ball::setSpeed(float newSpeed) {
        speed = newSpeed;
        float currentSpeed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        if (currentSpeed > 0) {
            velocity.x = velocity.x / currentSpeed * speed;
            velocity.y = velocity.y / currentSpeed * speed;
        }
    }

    void Ball::reset() {
        position = sf::Vector2f(WINDOW_WIDTH / 2.0f - size.x / 2, WINDOW_HEIGHT / 2.0f - size.y / 2);
        resetDirection();
        sprite.setPosition(position);
    }

}