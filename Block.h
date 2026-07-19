#pragma once
#include "GameObject.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>

namespace Arcanoid {

    class Block : public GameObject {
    protected:
        bool isActive;
        sf::Texture texture;
        sf::Color color;

    public:
        Block(float x, float y, sf::Color color = sf::Color::White);
        virtual ~Block() = default;

        void update(float dt) override;
        virtual void draw(sf::RenderWindow& window) const override;

        virtual bool isAlive() const { return isActive; }
        virtual void destroy() { isActive = false; }
        virtual sf::Color getColor() const { return color; }
    };

}