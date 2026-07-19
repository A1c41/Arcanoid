#pragma once
#include "GameObject.h"
#include "Constants.h"

namespace Arcanoid {

    class Ball : public GameObject {
    private:
        sf::Vector2f velocity;
        float speed;
        sf::Texture texture;

        void resetDirection();

    public:
        Ball(float initialSpeed = 400.0f);
        ~Ball() = default;

        void update(float dt) override;
        void draw(sf::RenderWindow& window) const override;

        void bounceFromPaddle(const GameObject& paddle);
        bool isOutOfBounds() const;
        void setSpeed(float newSpeed);
        void reset();

        sf::Vector2f getVelocity() const { return velocity; }
        void setVelocity(const sf::Vector2f& newVelocity) { velocity = newVelocity; }
        void reflectX() { velocity.x = -velocity.x; }
        void reflectY() { velocity.y = -velocity.y; }
    };

}