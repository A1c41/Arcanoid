#pragma once
#include "GameObject.h"
#include "Constants.h"

namespace Arcanoid {

    class Paddle : public GameObject {
    private:
        float speed;
        bool moveLeft;
        bool moveRight;
        sf::Texture texture;
        float baseWidth;
        float baseHeight;

    public:
        Paddle();
        ~Paddle() = default;

        void update(float dt) override;
        void draw(sf::RenderWindow& window) const override;
        void handleInput(const sf::Event& event) override;

        void setSpeed(float newSpeed) { speed = newSpeed; }
        float getSpeed() const { return speed; }
        void setSize(const sf::Vector2f& newSize) override;
        float getBaseWidth() const { return baseWidth; }
        float getBaseHeight() const { return baseHeight; }

        sf::FloatRect getBounds() const override;
    };

}