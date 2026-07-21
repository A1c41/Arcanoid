// SaveSystem.cpp
#include "SaveSystem.h"
#include "Ball.h"
#include "Paddle.h"
#include "Block.h"
#include "StrongBlock.h"
#include "GlassBlock.h"
#include "IndestructibleBlock.h"
#include <fstream>
#include <cstdio>

namespace Arcanoid {

    GameMemento::GameMemento() : score(0), lives(3), difficulty(1) {}

    void GameMemento::saveState(int score_, int lives_, int difficulty_,
        const std::vector<std::unique_ptr<GameObject>>& blocks_,
        const GameObject* ball_,
        const GameObject* paddle_,
        const std::vector<std::unique_ptr<Bonus>>& bonuses_,
        const std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>>& activeEffects_) {
        score = score_;
        lives = lives_;
        difficulty = difficulty_;

        blocks.clear();
        for (const auto& block : blocks_) {
            BlockState state;
            state.x = block->getPosition().x;
            state.y = block->getPosition().y;
            state.isActive = true;
            state.hitsRemaining = 0;
            state.maxHits = 0;
            state.type = BlockState::NORMAL;
            state.colorR = 255;
            state.colorG = 255;
            state.colorB = 255;

            auto* indestructibleBlock = dynamic_cast<IndestructibleBlock*>(block.get());
            if (indestructibleBlock) {
                state.type = BlockState::INDESTRUCTIBLE;
                state.isActive = true;
                sf::Color color = indestructibleBlock->getColor();
                state.colorR = color.r;
                state.colorG = color.g;
                state.colorB = color.b;
                blocks.push_back(state);
                continue;
            }

            auto* glassBlock = dynamic_cast<GlassBlock*>(block.get());
            if (glassBlock) {
                state.type = BlockState::GLASS;
                state.isActive = glassBlock->isAlive();
                sf::Color color = glassBlock->getColor();
                state.colorR = color.r;
                state.colorG = color.g;
                state.colorB = color.b;
                blocks.push_back(state);
                continue;
            }

            auto* strongBlock = dynamic_cast<StrongBlock*>(block.get());
            if (strongBlock) {
                state.type = BlockState::STRONG;
                state.isActive = strongBlock->isAlive();
                state.hitsRemaining = strongBlock->getHitsRemaining();
                state.maxHits = strongBlock->getMaxHits();
                sf::Color color = strongBlock->getColor();
                state.colorR = color.r;
                state.colorG = color.g;
                state.colorB = color.b;
                blocks.push_back(state);
                continue;
            }

            auto* normalBlock = dynamic_cast<Block*>(block.get());
            if (normalBlock) {
                state.type = BlockState::NORMAL;
                state.isActive = normalBlock->isAlive();
                sf::Color color = normalBlock->getColor();
                state.colorR = color.r;
                state.colorG = color.g;
                state.colorB = color.b;
            }

            blocks.push_back(state);
        }

        if (ball_) {
            auto* ball = dynamic_cast<const Ball*>(ball_);
            if (ball) {
                ballState.x = ball->getPosition().x;
                ballState.y = ball->getPosition().y;
                ballState.vx = ball->getVelocity().x;
                ballState.vy = ball->getVelocity().y;
                ballState.speed = ball->getSpeed();
            }
        }

        if (paddle_) {
            paddleState.x = paddle_->getPosition().x;
            paddleState.y = paddle_->getPosition().y;
            paddleState.sizeX = paddle_->getSize().x;
            paddleState.sizeY = paddle_->getSize().y;
            auto* paddle = dynamic_cast<const Paddle*>(paddle_);
            if (paddle) {
                paddleState.speed = paddle->getSpeed();
            }
        }

        bonuses.clear();
        for (const auto& bonus : bonuses_) {
            if (bonus->isAlive()) {
                BonusState state;
                state.x = bonus->getPosition().x;
                state.y = bonus->getPosition().y;
                state.type = static_cast<int>(bonus->getType());
                state.isActive = true;
                bonuses.push_back(state);
            }
        }

        activeEffects.clear();
        for (const auto& effect : activeEffects_) {
            ActiveEffectState state;
            state.type = effect.first->getType();
            state.elapsedTime = effect.second;
            state.multiplier = effect.first->getMultiplier();
            activeEffects.push_back(state);
        }
    }

    int GameMemento::getScore() const { return score; }
    int GameMemento::getLives() const { return lives; }
    int GameMemento::getDifficulty() const { return difficulty; }
    const std::vector<GameMemento::BlockState>& GameMemento::getBlocks() const { return blocks; }
    const GameMemento::BallState& GameMemento::getBall() const { return ballState; }
    const GameMemento::PaddleState& GameMemento::getPaddle() const { return paddleState; }
    const std::vector<GameMemento::BonusState>& GameMemento::getBonuses() const { return bonuses; }
    const std::vector<GameMemento::ActiveEffectState>& GameMemento::getActiveEffects() const { return activeEffects; }

    SaveSystem::SaveSystem() {
        saveFile = RESOURCES + "save.dat";
        loadFromFile();
    }

    SaveSystem::~SaveSystem() {
        if (save) {
            saveToFile();
        }
    }

    void SaveSystem::saveGame(int score, int lives, int difficulty,
        const std::vector<std::unique_ptr<GameObject>>& blocks,
        const GameObject* ball,
        const GameObject* paddle,
        const std::vector<std::unique_ptr<Bonus>>& bonuses,
        const std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>>& activeEffects) {
        save = std::make_unique<GameMemento>();
        save->saveState(score, lives, difficulty, blocks, ball, paddle, bonuses, activeEffects);
        saveToFile();
    }

    bool SaveSystem::loadGame() {
        loadFromFile();
        return save != nullptr;
    }

    const GameMemento* SaveSystem::getCurrentSave() const {
        return save.get();
    }

    bool SaveSystem::hasSave() const {
        return save != nullptr;
    }

    void SaveSystem::clearSave() {
        save.reset();
        std::remove(saveFile.c_str());
    }

    void SaveSystem::saveToFile() {
        if (!save) return;

        std::ofstream file(saveFile, std::ios::binary);
        if (!file.is_open()) return;

        int score = save->getScore();
        int lives = save->getLives();
        int difficulty = save->getDifficulty();
        file.write(reinterpret_cast<const char*>(&score), sizeof(int));
        file.write(reinterpret_cast<const char*>(&lives), sizeof(int));
        file.write(reinterpret_cast<const char*>(&difficulty), sizeof(int));

        GameMemento::BallState ballState = save->getBall();
        file.write(reinterpret_cast<const char*>(&ballState), sizeof(GameMemento::BallState));

        GameMemento::PaddleState paddleState = save->getPaddle();
        file.write(reinterpret_cast<const char*>(&paddleState), sizeof(GameMemento::PaddleState));

        std::vector<GameMemento::BlockState> blocks = save->getBlocks();
        size_t blockCount = blocks.size();
        file.write(reinterpret_cast<const char*>(&blockCount), sizeof(size_t));
        for (const auto& block : blocks) {
            file.write(reinterpret_cast<const char*>(&block), sizeof(GameMemento::BlockState));
        }

        std::vector<GameMemento::BonusState> bonuses = save->getBonuses();
        size_t bonusCount = bonuses.size();
        file.write(reinterpret_cast<const char*>(&bonusCount), sizeof(size_t));
        for (const auto& bonus : bonuses) {
            file.write(reinterpret_cast<const char*>(&bonus), sizeof(GameMemento::BonusState));
        }

        std::vector<GameMemento::ActiveEffectState> activeEffects = save->getActiveEffects();
        size_t effectCount = activeEffects.size();
        file.write(reinterpret_cast<const char*>(&effectCount), sizeof(size_t));
        for (const auto& effect : activeEffects) {
            int type = effect.type;
            float elapsedTime = effect.elapsedTime;
            float multiplier = effect.multiplier;

            file.write(reinterpret_cast<const char*>(&type), sizeof(int));
            file.write(reinterpret_cast<const char*>(&elapsedTime), sizeof(float));
            file.write(reinterpret_cast<const char*>(&multiplier), sizeof(float));
        }

        file.close();
    }

    void SaveSystem::loadFromFile() {
        std::ifstream file(saveFile, std::ios::binary);
        if (!file.is_open()) {
            save = nullptr;
            return;
        }

        try {
            save = std::make_unique<GameMemento>();

            int score, lives, difficulty;
            file.read(reinterpret_cast<char*>(&score), sizeof(int));
            file.read(reinterpret_cast<char*>(&lives), sizeof(int));
            file.read(reinterpret_cast<char*>(&difficulty), sizeof(int));
            save->setScore(score);
            save->setLives(lives);
            save->setDifficulty(difficulty);

            GameMemento::BallState ballState;
            file.read(reinterpret_cast<char*>(&ballState), sizeof(GameMemento::BallState));
            save->setBall(ballState);

            GameMemento::PaddleState paddleState;
            file.read(reinterpret_cast<char*>(&paddleState), sizeof(GameMemento::PaddleState));
            save->setPaddle(paddleState);

            size_t blockCount;
            file.read(reinterpret_cast<char*>(&blockCount), sizeof(size_t));
            std::vector<GameMemento::BlockState> blocks;
            for (size_t i = 0; i < blockCount; ++i) {
                GameMemento::BlockState block;
                file.read(reinterpret_cast<char*>(&block), sizeof(GameMemento::BlockState));
                blocks.push_back(block);
            }
            save->setBlocks(blocks);

            size_t bonusCount;
            file.read(reinterpret_cast<char*>(&bonusCount), sizeof(size_t));
            std::vector<GameMemento::BonusState> bonuses;
            for (size_t i = 0; i < bonusCount; ++i) {
                GameMemento::BonusState bonus;
                file.read(reinterpret_cast<char*>(&bonus), sizeof(GameMemento::BonusState));
                bonuses.push_back(bonus);
            }
            save->setBonuses(bonuses);

            size_t effectCount;
            file.read(reinterpret_cast<char*>(&effectCount), sizeof(size_t));
            std::vector<GameMemento::ActiveEffectState> activeEffects;
            for (size_t i = 0; i < effectCount; ++i) {
                GameMemento::ActiveEffectState effect;
                int type;
                float elapsedTime;
                float multiplier;

                file.read(reinterpret_cast<char*>(&type), sizeof(int));
                file.read(reinterpret_cast<char*>(&elapsedTime), sizeof(float));
                file.read(reinterpret_cast<char*>(&multiplier), sizeof(float));

                effect.type = type;
                effect.elapsedTime = elapsedTime;
                effect.multiplier = multiplier;

                activeEffects.push_back(effect);
            }
            save->setActiveEffects(activeEffects);

            file.close();
        }
        catch (...) {
            save = nullptr;
            file.close();
        }
    }

}