#include "StrongBlock.h"

namespace Arcanoid {

    StrongBlock::StrongBlock(float x, float y, sf::Color color, int hits)
        : Block(x, y, color), maxHits(hits), hitsRemaining(hits) {

        for (int i = 0; i < hits; ++i) {
            float factor = 1.0f - (i / (float)hits) * 0.6f;
            sf::Color c = color;
            c.r = static_cast<sf::Uint8>(c.r * factor);
            c.g = static_cast<sf::Uint8>(c.g * factor);
            c.b = static_cast<sf::Uint8>(c.b * factor);
            colors.push_back(c);
        }

        if (!font.loadFromFile(RESOURCES + "fonts\\Roboto-Black.ttf")) {
            font.loadFromFile("C:/Windows/Fonts/arial.ttf");
        }

        hitCountText.setFont(font);
        hitCountText.setCharacterSize(16);
        hitCountText.setFillColor(sf::Color::White);
        hitCountText.setString(std::to_string(hitsRemaining));
        hitCountText.setPosition(position.x + size.x / 2 - 8, position.y + size.y / 2 - 10);

        updateAppearance();
    }

    void StrongBlock::updateAppearance() {
        if (hitsRemaining > 0 && hitsRemaining <= static_cast<int>(colors.size())) {
            this->color = colors[hitsRemaining - 1];
            sf::Image image;
            image.create(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y), color);

            for (int px = 0; px < static_cast<int>(size.x); ++px) {
                for (int py = 0; py < static_cast<int>(size.y); ++py) {
                    if (px == 0 || px == static_cast<int>(size.x) - 1 ||
                        py == 0 || py == static_cast<int>(size.y) - 1) {
                        image.setPixel(px, py, sf::Color::White);
                    }
                }
            }

            texture.loadFromImage(image);
            sprite.setTexture(texture);
            sprite.setPosition(position);

            hitCountText.setString(std::to_string(hitsRemaining));
            hitCountText.setPosition(position.x + size.x / 2 - 8, position.y + size.y / 2 - 10);
        }
    }

    void StrongBlock::hit() {
        if (hitsRemaining > 0) {
            hitsRemaining--;
            if (hitsRemaining == 0) {
                isActive = false;
            }
            else {
                updateAppearance();
            }
        }
    }

    void StrongBlock::setHitsRemaining(int hits) {
        hitsRemaining = hits;
        if (hitsRemaining > 0) {
            isActive = true;
            updateAppearance();
        }
        else {
            isActive = false;
        }
    }

    sf::Color StrongBlock::getColor() const {
        return color;
    }

    void StrongBlock::draw(sf::RenderWindow& window) const {
        if (isActive) {
            window.draw(sprite);
            window.draw(hitCountText);
        }
    }

}