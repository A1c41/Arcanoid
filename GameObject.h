#pragma once
#include <SFML/Graphics.hpp>

namespace Arcanoid {

    class GameObject {
    protected:
        sf::Vector2f position;
        sf::Vector2f size;
        sf::Sprite sprite;

    public:
        virtual ~GameObject() = default;
        virtual void update(float dt) = 0;
        virtual void draw(sf::RenderWindow& window) const = 0;
        virtual void handleInput(const sf::Event& event) {}

        virtual sf::FloatRect getBounds() const {
            return sprite.getGlobalBounds();
        }

        void setPosition(float x, float y) {
            position = sf::Vector2f(x, y);
            sprite.setPosition(x, y);
        }

        virtual void setSize(const sf::Vector2f& newSize) {
            size = newSize;
        }

        sf::Vector2f getPosition() const { return position; }
        sf::Vector2f getSize() const { return size; }
    };

}