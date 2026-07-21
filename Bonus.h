// Bonus.h
#pragma once
#include "GameObject.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <utility>
#include <stack>

namespace Arcanoid {

    class Paddle;
    class Ball;
    class Block;
    class StrongBlock;

    class IBonusEffect {
    public:
        virtual ~IBonusEffect() = default;
        virtual void apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) = 0;
        virtual void remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) = 0;
        virtual float getDuration() const = 0;
        virtual sf::Color getColor() const = 0;
        virtual int getType() const = 0;
        virtual void restoreState(float elapsedTime) = 0;
        virtual float getElapsedTime() const = 0;
        virtual void reset() = 0;
    };

    class FireBallEffect : public IBonusEffect {
    private:
        float duration;
        float elapsedTime;
        bool isActive;
        float speedMultiplier;
        int activeCount;
        std::stack<float> speedStack;

    public:
        FireBallEffect();
        void apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        void remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        float getDuration() const override;
        sf::Color getColor() const override;
        int getType() const override;
        void restoreState(float elapsedTime) override;
        float getElapsedTime() const override;
        void reset() override;
        bool isFireBallActive() const { return isActive; }
    };

    class FragileBlocksEffect : public IBonusEffect {
    private:
        float duration;
        float elapsedTime;
        bool isApplied;
        std::vector<std::pair<StrongBlock*, int>> affectedBlocks;
        std::stack<std::vector<std::pair<StrongBlock*, int>>> stateStack;
        int activeCount;

    public:
        FragileBlocksEffect();
        void apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        void remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        float getDuration() const override;
        sf::Color getColor() const override;
        int getType() const override;
        void restoreState(float elapsedTime) override;
        float getElapsedTime() const override;
        void reset() override;
        const std::vector<std::pair<StrongBlock*, int>>& getAffectedBlocks() const { return affectedBlocks; }
        void setAffectedBlocks(const std::vector<std::pair<StrongBlock*, int>>& blocks) { affectedBlocks = blocks; }
    };

    class PaddleSizeEffect : public IBonusEffect {
    private:
        float duration;
        float elapsedTime;
        float sizeMultiplier;
        bool isApplied;
        std::stack<sf::Vector2f> sizeStack;
        int activeCount;

    public:
        PaddleSizeEffect(float multiplier);
        void apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        void remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        float getDuration() const override;
        sf::Color getColor() const override;
        float getMultiplier() const;
        int getType() const override;
        void restoreState(float elapsedTime) override;
        float getElapsedTime() const override;
        void reset() override;
    };

    class PaddleSpeedEffect : public IBonusEffect {
    private:
        float duration;
        float elapsedTime;
        float speedMultiplier;
        bool isApplied;
        std::stack<float> speedStack;
        int activeCount;

    public:
        PaddleSpeedEffect(float multiplier);
        void apply(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        void remove(Paddle* paddle, Ball* ball, std::vector<std::unique_ptr<GameObject>>& blocks) override;
        float getDuration() const override;
        sf::Color getColor() const override;
        float getMultiplier() const;
        int getType() const override;
        void restoreState(float elapsedTime) override;
        float getElapsedTime() const override;
        void reset() override;
    };

    class Bonus : public GameObject {
    public:
        enum class Type {
            FIRE_BALL = 0,
            FRAGILE_BLOCKS = 1,
            SIZE_UP = 2,
            SIZE_DOWN = 3,
            SPEED_UP = 4,
            SPEED_DOWN = 5
        };

    private:
        Type type;
        std::unique_ptr<IBonusEffect> effect;
        float fallSpeed;
        bool isActive;
        sf::CircleShape shape;

        void createShape();

    public:
        Bonus(float x, float y, Type type);
        ~Bonus() = default;

        void update(float dt) override;
        void draw(sf::RenderWindow& window) const override;

        bool isAlive() const { return isActive; }
        void collect() { isActive = false; }
        Type getType() const { return type; }
        IBonusEffect* getEffect() { return effect.get(); }
        std::unique_ptr<IBonusEffect> releaseEffect() { return std::move(effect); }

        sf::FloatRect getBounds() const override;
    };

    class BonusFactory {
    public:
        static std::unique_ptr<Bonus> createRandomBonus(float x, float y);
    };

}