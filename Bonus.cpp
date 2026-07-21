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
        isActive(false), speedMultiplier(1.5f), activeCount(0) {
    }

    void FireBallEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (ball) {
            float currentSpeed = ball->getSpeed();
            speedStack.push(currentSpeed);
            ball->setSpeed(currentSpeed * speedMultiplier);
            isActive = true;
            activeCount++;
        }
    }

    void FireBallEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (ball && !speedStack.empty()) {
            float previousSpeed = speedStack.top();
            speedStack.pop();
            ball->setSpeed(previousSpeed);
            activeCount--;
            if (activeCount <= 0) {
                isActive = false;
                while (!speedStack.empty()) speedStack.pop();
                activeCount = 0;
            }
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
        activeCount = 0;
        while (!speedStack.empty()) speedStack.pop();
    }

    FragileBlocksEffect::FragileBlocksEffect() : duration(10.0f), elapsedTime(0.0f), isApplied(false), activeCount(0) {}

    void FragileBlocksEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        std::vector<std::pair<StrongBlock*, int>> currentState;
        for (auto& block : blocks) {
            auto* strongBlock = dynamic_cast<StrongBlock*>(block.get());
            if (strongBlock && strongBlock->isAlive()) {
                int originalHits = strongBlock->getHitsRemaining();
                if (originalHits > 1) {
                    currentState.push_back({ strongBlock, originalHits });
                    strongBlock->setHitsRemaining(1);
                    strongBlock->updateAppearance();
                }
            }
        }
        if (!currentState.empty()) {
            stateStack.push(currentState);
            affectedBlocks = currentState;
            isApplied = true;
        }
        activeCount++;
        elapsedTime = 0.0f;
    }

    void FragileBlocksEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        activeCount--;
        if (activeCount <= 0 && !stateStack.empty()) {
            auto previousState = stateStack.top();
            stateStack.pop();

            for (auto& affected : previousState) {
                if (affected.first && affected.first->isAlive()) {
                    affected.first->setHitsRemaining(affected.second);
                    affected.first->updateAppearance();
                }
            }

            if (stateStack.empty()) {
                affectedBlocks.clear();
                isApplied = false;
                activeCount = 0;
            }
            else {
                affectedBlocks = stateStack.top();
            }
        }
    }

    float FragileBlocksEffect::getDuration() const { return duration; }
    sf::Color FragileBlocksEffect::getColor() const { return sf::Color(100, 255, 100); }
    int FragileBlocksEffect::getType() const { return 1; }
    void FragileBlocksEffect::restoreState(float time) { elapsedTime = time; }
    float FragileBlocksEffect::getElapsedTime() const { return elapsedTime; }
    void FragileBlocksEffect::reset() {
        affectedBlocks.clear();
        while (!stateStack.empty()) stateStack.pop();
        isApplied = false;
        elapsedTime = 0.0f;
        activeCount = 0;
    }

    PaddleSizeEffect::PaddleSizeEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), sizeMultiplier(multiplier), isApplied(false), activeCount(0) {
    }

    void PaddleSizeEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle) {
            sf::Vector2f currentSize = paddle->getSize();
            sizeStack.push(currentSize);
            sf::Vector2f newSize(currentSize.x * sizeMultiplier, currentSize.y);
            paddle->setSize(newSize);
            isApplied = true;
            activeCount++;
        }
    }

    void PaddleSizeEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied && !sizeStack.empty()) {
            activeCount--;
            if (activeCount <= 0) {
                sf::Vector2f previousSize = sizeStack.top();
                sizeStack.pop();
                paddle->setSize(previousSize);
                while (!sizeStack.empty()) {
                    sizeStack.pop();
                }
                isApplied = false;
                activeCount = 0;
            }
            else {
                sizeStack.pop();
            }
        }
    }

    float PaddleSizeEffect::getDuration() const { return duration; }
    sf::Color PaddleSizeEffect::getColor() const {
        return sizeMultiplier > 1.0f ? sf::Color(100, 255, 255) : sf::Color(255, 100, 255);
    }
    float PaddleSizeEffect::getMultiplier() const { return sizeMultiplier; }
    int PaddleSizeEffect::getType() const { return sizeMultiplier > 1.0f ? 2 : 3; }
    void PaddleSizeEffect::restoreState(float time) { elapsedTime = time; }
    float PaddleSizeEffect::getElapsedTime() const { return elapsedTime; }
    void PaddleSizeEffect::reset() {
        while (!sizeStack.empty()) sizeStack.pop();
        isApplied = false;
        elapsedTime = 0.0f;
        activeCount = 0;
    }

    PaddleSpeedEffect::PaddleSpeedEffect(float multiplier)
        : duration(10.0f), elapsedTime(0.0f), speedMultiplier(multiplier), isApplied(false), activeCount(0) {
    }

    void PaddleSpeedEffect::apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle) {
            float currentSpeed = paddle->getSpeed();
            speedStack.push(currentSpeed);
            paddle->setSpeed(currentSpeed * speedMultiplier);
            isApplied = true;
            activeCount++;
        }
    }

    void PaddleSpeedEffect::remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) {
        if (paddle && isApplied && !speedStack.empty()) {
            activeCount--;
            float previousSpeed = speedStack.top();
            speedStack.pop();
            if (activeCount <= 0) {
                paddle->setSpeed(previousSpeed);
                while (!speedStack.empty()) speedStack.pop();
                isApplied = false;
                activeCount = 0;
            }
        }
    }

    float PaddleSpeedEffect::getDuration() const { return duration; }
    sf::Color PaddleSpeedEffect::getColor() const {
        return speedMultiplier > 1.0f ? sf::Color(255, 255, 100) : sf::Color(255, 100, 100);
    }
    float PaddleSpeedEffect::getMultiplier() const { return speedMultiplier; }
    int PaddleSpeedEffect::getType() const { return speedMultiplier > 1.0f ? 4 : 5; }
    void PaddleSpeedEffect::restoreState(float time) { elapsedTime = time; }
    float PaddleSpeedEffect::getElapsedTime() const { return elapsedTime; }
    void PaddleSpeedEffect::reset() {
        while (!speedStack.empty()) speedStack.pop();
        isApplied = false;
        elapsedTime = 0.0f;
        activeCount = 0;
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