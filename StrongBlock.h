#pragma once
#include "Block.h"
#include "Constants.h"
#include <vector>

namespace Arcanoid {

    class StrongBlock : public Block {
    private:
        int maxHits;
        int hitsRemaining;
        std::vector<sf::Color> colors;
        sf::Text hitCountText;
        sf::Font font;

        void updateAppearance();

    public:
        StrongBlock(float x, float y, sf::Color color, int hits = 3);
        ~StrongBlock() = default;

        void draw(sf::RenderWindow& window) const override;
        void hit();
        bool isAlive() const override { return hitsRemaining > 0; }
        int getHitsRemaining() const { return hitsRemaining; }
        void setHitsRemaining(int hits);
        sf::Color getColor() const override;
    };

}