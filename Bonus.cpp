// Bonus.cpp
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

    FireBallEffect::FireBallEffect() : duration(8.0f), elapsedTime(0.0f),
        isActive(false), speedMultiplier(1.5f) {
    }

    void FireBallEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (ball && !isActive) {
            float currentSpeed = ball->getSpeed();
            originalSpeed = currentSpeed;
            ball->setSpeed(currentSpeed * speedMultiplier);
            isActive = true;
        }
        elapsedTime = 0.0f;
    }

    void FireBallEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (ball && isActive) {
            ball->setSpeed(originalSpeed);
            isActive = false;
            elapsedTime = 0.0f;
        }
    }

    float FireBallEffect::getDuration() const { return duration; }
    sf::Color FireBallEffect::getColor() const { return sf::Color(255, 100, 0); }
    int FireBallEffect::getType() const { return 0; }
    void FireBallEffect::restoreState(float time) { elapsedTime = time; }
    float FireBallEffect::getElapsedTime() const { return elapsedTime; }
    void FireBallEffect::reset() {
        isActive = false;
        elapsedTime = 0.0f;
    }

    FragileBlocksEffect::FragileBlocksEffect() : duration(10.0f), elapsedTime(0.0f), isApplied(false) {}

    void FragileBlocksEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (!isApplied) {
            for (auto& block : blocks) {
                auto* strongBlock = dynamic_cast<StrongBlock*>(block.get());
                if (strongBlock && strongBlock->isAlive()) {
                    int originalHits = strongBlock->getHitsRemaining();
                    if (originalHits > 1) {
                        affectedBlocks.push_back({ strongBlock, originalHits });
                        strongBlock->setHitsRemaining(1);
                        strongBlock->updateAppearance();
                    }
                }
            }
            if (!affectedBlocks.empty()) {
                isApplied = true;
            }
        }
        elapsedTime = 0.0f;
    }

    void FragileBlocksEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (isApplied) {
            for (auto& affected : affectedBlocks) {
                if (affected.first && affected.first->isAlive()) {
                    affected.first->setHitsRemaining(affected.second);
                    affected.first->updateAppearance();
                }
            }
            affectedBlocks.clear();
            isApplied = false;
            elapsedTime = 0.0f;
        }
    }

    float FragileBlocksEffect::getDuration() const { return duration; }
    sf::Color FragileBlocksEffect::getColor() const { return sf::Color(100, 255, 100); }
    int FragileBlocksEffect::getType() const { return 1; }
    void FragileBlocksEffect::restoreState(float time) { elapsedTime = time; }
    float FragileBlocksEffect::getElapsedTime() const { return elapsedTime; }
    void FragileBlocksEffect::reset() {
        affectedBlocks.clear();
        isApplied = false;
        elapsedTime = 0.0f;
    }

    PaddleSizeEffect::PaddleSizeEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), sizeMultiplier(multiplier), isApplied(false) {
    }

    void PaddleSizeEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && !isApplied) {
            originalSize = paddle->getSize();
            sf::Vector2f newSize(originalSize.x * sizeMultiplier, originalSize.y);
            paddle->setSize(newSize);
            isApplied = true;
        }
        elapsedTime = 0.0f;
    }

    void PaddleSizeEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied) {
            paddle->setSize(originalSize);
            isApplied = false;
            elapsedTime = 0.0f;
        }
    }

    float PaddleSizeEffect::getDuration() const { return duration; }
    sf::Color PaddleSizeEffect::getColor() const {
        return sizeMultiplier > 1.0f ? sf::Color(100, 255, 255) : sf::Color(255, 100, 255);
    }
    int PaddleSizeEffect::getType() const { return sizeMultiplier > 1.0f ? 2 : 3; }
    void PaddleSizeEffect::restoreState(float time) { elapsedTime = time; }
    float PaddleSizeEffect::getElapsedTime() const { return elapsedTime; }
    void PaddleSizeEffect::reset() {
        isApplied = false;
        elapsedTime = 0.0f;
    }

    PaddleSpeedEffect::PaddleSpeedEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), speedMultiplier(multiplier), isApplied(false) {
    }

    void PaddleSpeedEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && !isApplied) {
            originalSpeed = paddle->getSpeed();
            paddle->setSpeed(originalSpeed * speedMultiplier);
            isApplied = true;
        }
        elapsedTime = 0.0f;
    }

    void PaddleSpeedEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied) {
            paddle->setSpeed(originalSpeed);
            isApplied = false;
            elapsedTime = 0.0f;
        }
    }

    float PaddleSpeedEffect::getDuration() const { return duration; }
    sf::Color PaddleSpeedEffect::getColor() const {
        return speedMultiplier > 1.0f ? sf::Color(255, 255, 100) : sf::Color(255, 100, 100);
    }
    int PaddleSpeedEffect::getType() const { return speedMultiplier > 1.0f ? 4 : 5; }
    void PaddleSpeedEffect::restoreState(float time) { elapsedTime = time; }
    float PaddleSpeedEffect::getElapsedTime() const { return elapsedTime; }
    void PaddleSpeedEffect::reset() {
        isApplied = false;
        elapsedTime = 0.0f;
    }

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