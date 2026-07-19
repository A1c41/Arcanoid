#pragma once
#include "Block.h"

namespace Arcanoid {

    class IndestructibleBlock : public Block {
    private:
        bool isActive;
        sf::Texture texture;

    public:
        IndestructibleBlock(float x, float y);
        ~IndestructibleBlock() = default;

        void draw(sf::RenderWindow& window) const override;
        bool isAlive() const override { return true; }
        void destroy() override {}
        sf::Color getColor() const override { return sf::Color(80, 80, 80); }
    };

}