#pragma once
#include "Block.h"

namespace Arcanoid {

    class GlassBlock : public Block {
    private:
        bool isActive;
        sf::Texture texture;

    public:
        GlassBlock(float x, float y);
        ~GlassBlock() = default;

        void draw(sf::RenderWindow& window) const override;
        void destroy() override { isActive = false; }
        bool isAlive() const override { return isActive; }
        sf::Color getColor() const override { return sf::Color(200, 230, 255, 180); }
    };

}