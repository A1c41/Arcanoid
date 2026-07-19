#include "Bonus.h"
#include "Paddle.h"
#include "Ball.h"
#include "Block.h"
#include "StrongBlock.h"
#include "GlassBlock.h"
#include "IndestructibleBlock.h"
#include <cstdlib>
#include <cmath>

namespace Arcanoid {

    FireBallEffect::FireBallEffect() : duration(8.0f), elapsedTime(0.0f), isActive(false), baseSpeed(0), speedStored(false) {}

    void FireBallEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (ball && !isActive) {
            if (!speedStored) {
                sf::Vector2f vel = ball->getVelocity();
                baseSpeed = std::sqrt(vel.x * vel.x + vel.y * vel.y);
                speedStored = true;
            }
            ball->setSpeed(baseSpeed * 1.5f);
            isActive = true;
        }
    }

    void FireBallEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (isActive && ball && speedStored) {
            ball->setSpeed(baseSpeed);
            isActive = false;
        }
    }

    float FireBallEffect::getDuration() const { return duration; }
    sf::Color FireBallEffect::getColor() const { return sf::Color(255, 100, 0); }
    bool FireBallEffect::isFireBallActive() const { return isActive; }
    int FireBallEffect::getType() const { return 0; }

    void FireBallEffect::restoreState(float time) {
        elapsedTime = time;
        isActive = true;
    }

    float FireBallEffect::getElapsedTime() const { return elapsedTime; }

    FragileBlocksEffect::FragileBlocksEffect() : duration(10.0f), elapsedTime(0.0f), isApplied(false) {}

    void FragileBlocksEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (isApplied) return;

        affectedBlocks.clear();
        for (auto& block : blocks) {
            auto* strongBlock = dynamic_cast<StrongBlock*>(block.get());
            if (strongBlock && strongBlock->isAlive()) {
                int originalHits = strongBlock->getHitsRemaining();
                if (originalHits > 1) {
                    affectedBlocks.push_back({ strongBlock, originalHits });
                    strongBlock->setHitsRemaining(1);
                }
            }
        }
        isApplied = true;
    }

    void FragileBlocksEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        for (auto& affected : affectedBlocks) {
            if (affected.first && affected.first->isAlive()) {
                affected.first->setHitsRemaining(affected.second);
            }
        }
        affectedBlocks.clear();
        isApplied = false;
    }

    float FragileBlocksEffect::getDuration() const { return duration; }
    sf::Color FragileBlocksEffect::getColor() const { return sf::Color(100, 255, 100); }
    int FragileBlocksEffect::getType() const { return 1; }

    void FragileBlocksEffect::restoreState(float time) {
        elapsedTime = time;
        isApplied = true;
    }

    float FragileBlocksEffect::getElapsedTime() const { return elapsedTime; }

    PaddleSizeEffect::PaddleSizeEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), sizeMultiplier(multiplier), isApplied(false) {
    }

    void PaddleSizeEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && !isApplied) {
            isApplied = true;
            sf::Vector2f currentSize = paddle->getSize();
            sf::Vector2f newSize = currentSize;
            newSize.x = currentSize.x * sizeMultiplier;
            paddle->setSize(newSize);
        }
    }

    void PaddleSizeEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied) {
            isApplied = false;
            sf::Vector2f currentSize = paddle->getSize();
            sf::Vector2f newSize = currentSize;
            newSize.x = currentSize.x / sizeMultiplier;
            paddle->setSize(newSize);
        }
    }

    float PaddleSizeEffect::getDuration() const { return duration; }
    sf::Color PaddleSizeEffect::getColor() const {
        return sizeMultiplier > 1.0f ? sf::Color(100, 255, 255) : sf::Color(255, 100, 255);
    }
    float PaddleSizeEffect::getMultiplier() const { return sizeMultiplier; }
    int PaddleSizeEffect::getType() const { return sizeMultiplier > 1.0f ? 2 : 3; }

    void PaddleSizeEffect::restoreState(float time) {
        elapsedTime = time;
        isApplied = true;
    }

    float PaddleSizeEffect::getElapsedTime() const { return elapsedTime; }

    PaddleSpeedEffect::PaddleSpeedEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), speedMultiplier(multiplier), isApplied(false) {
    }

    void PaddleSpeedEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && !isApplied) {
            isApplied = true;
            paddle->setSpeed(paddle->getSpeed() * speedMultiplier);
        }
    }

    void PaddleSpeedEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied) {
            isApplied = false;
            paddle->setSpeed(paddle->getSpeed() / speedMultiplier);
        }
    }

    float PaddleSpeedEffect::getDuration() const { return duration; }
    sf::Color PaddleSpeedEffect::getColor() const {
        return speedMultiplier > 1.0f ? sf::Color(255, 255, 100) : sf::Color(255, 100, 100);
    }
    float PaddleSpeedEffect::getMultiplier() const { return speedMultiplier; }
    int PaddleSpeedEffect::getType() const { return speedMultiplier > 1.0f ? 4 : 5; }

    void PaddleSpeedEffect::restoreState(float time) {
        elapsedTime = time;
        isApplied = true;
    }

    float PaddleSpeedEffect::getElapsedTime() const { return elapsedTime; }

    Bonus::Bonus(float x, float y, Type type)
        : type(type), fallSpeed(150.0f), isActive(true), effect(nullptr) {

        position = sf::Vector2f(x, y);
        size = sf::Vector2f(20.0f, 20.0f);

        createShape();

        switch (type) {
        case Type::FIRE_BALL:
            effect = std::make_unique<FireBallEffect>();
            break;
        case Type::FRAGILE_BLOCKS:
            effect = std::make_unique<FragileBlocksEffect>();
            break;
        case Type::SIZE_UP:
            effect = std::make_unique<PaddleSizeEffect>(1.5f);
            break;
        case Type::SIZE_DOWN:
            effect = std::make_unique<PaddleSizeEffect>(0.7f);
            break;
        case Type::SPEED_UP:
            effect = std::make_unique<PaddleSpeedEffect>(1.5f);
            break;
        case Type::SPEED_DOWN:
            effect = std::make_unique<PaddleSpeedEffect>(0.7f);
            break;
        }
    }

    void Bonus::createShape() {
        shape.setRadius(10.0f);
        shape.setPosition(position);

        sf::Color color;
        switch (type) {
        case Type::FIRE_BALL: color = sf::Color(255, 100, 0); break;
        case Type::FRAGILE_BLOCKS: color = sf::Color(100, 255, 100); break;
        case Type::SIZE_UP: color = sf::Color(100, 255, 255); break;
        case Type::SIZE_DOWN: color = sf::Color(255, 100, 255); break;
        case Type::SPEED_UP: color = sf::Color(255, 255, 100); break;
        case Type::SPEED_DOWN: color = sf::Color(255, 100, 100); break;
        }
        shape.setFillColor(color);
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(2.0f);
    }

    void Bonus::update(float dt) {
        if (!isActive) return;

        position.y += fallSpeed * dt;
        shape.setPosition(position);

        if (position.y > WINDOW_HEIGHT) {
            isActive = false;
        }
    }

    void Bonus::draw(sf::RenderWindow& window) const {
        if (isActive) {
            window.draw(shape);
        }
    }

    sf::FloatRect Bonus::getBounds() const {
        return shape.getGlobalBounds();
    }

    std::unique_ptr<Bonus> BonusFactory::createRandomBonus(float x, float y) {
        int type = rand() % 6;
        return std::make_unique<Bonus>(x, y, static_cast<Bonus::Type>(type));
    }

}