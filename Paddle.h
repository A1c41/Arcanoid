#pragma once
#include "GameObject.h"
#include "Constants.h"

namespace Arcanoid {

    class Paddle : public GameObject {
    private:
        float speed;
        float baseSpeed;
        bool moveLeft;
        bool moveRight;
        sf::RectangleShape shape;
        float baseWidth;
        float baseHeight;

        static constexpr float DEFAULT_WIDTH = 120.0f;
        static constexpr float DEFAULT_HEIGHT = 22.0f;
        static constexpr float DEFAULT_SPEED = 350.0f;

    public:
        Paddle();
        ~Paddle() = default;

        void update(float dt) override;
        void draw(sf::RenderWindow& window) const override;
        void handleInput(const sf::Event& event) override;

        void setSpeed(float newSpeed) { speed = newSpeed; }
        float getSpeed() const { return speed; }
        float getBaseSpeed() const { return baseSpeed; }
        void setSize(const sf::Vector2f& newSize) override;
        float getBaseWidth() const { return baseWidth; }
        float getBaseHeight() const { return baseHeight; }
        void setBaseSpeed(float bs) { baseSpeed = bs; }

        sf::FloatRect getBounds() const override;
    };

}