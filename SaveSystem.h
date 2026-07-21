#pragma once
#include <memory>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "GameObject.h"
#include "Constants.h"
#include "Bonus.h"

namespace Arcanoid {

    class GameMemento {
    public:
        struct BlockState {
            float x;
            float y;
            bool isActive;
            int hitsRemaining;
            enum BlockType { NORMAL, STRONG, GLASS, INDESTRUCTIBLE } type;
            int colorR, colorG, colorB;

            BlockState() : x(0), y(0), isActive(true), hitsRemaining(0),
                type(NORMAL), colorR(255), colorG(255), colorB(255) {
            }
        };

        struct BallState {
            float x, y;
            float vx, vy;
            float speed;

            BallState() : x(0), y(0), vx(0), vy(0), speed(0) {}
        };

        struct PaddleState {
            float x, y;
            float speed;
            float sizeX, sizeY;

            PaddleState() : x(0), y(0), speed(0), sizeX(0), sizeY(0) {}
        };

        struct BonusState {
            float x, y;
            int type;
            float remainingTime;
            bool isActive;

            BonusState() : x(0), y(0), type(0), remainingTime(0), isActive(false) {}
        };

        struct AffectedBlockInfo {
            int blockIndex;
            int originalHits;

            AffectedBlockInfo() : blockIndex(-1), originalHits(0) {}
        };

        struct ActiveEffectState {
            int type;
            float elapsedTime;
            float multiplier;
            std::vector<AffectedBlockInfo> affectedBlocks;

            ActiveEffectState() : type(0), elapsedTime(0), multiplier(1.0f) {}
        };

    private:
        int score;
        int lives;
        int difficulty;
        std::vector<BlockState> blocks;
        BallState ballState;
        PaddleState paddleState;
        std::vector<BonusState> bonuses;
        std::vector<ActiveEffectState> activeEffects;

    public:
        GameMemento();

        void saveState(int score_, int lives_, int difficulty_,
            const std::vector<std::unique_ptr<GameObject>>& blocks_,
            const GameObject* ball_,
            const GameObject* paddle_,
            const std::vector<std::unique_ptr<Bonus>>& bonuses_,
            const std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>>& activeEffects_);

        int getScore() const;
        int getLives() const;
        int getDifficulty() const;
        const std::vector<BlockState>& getBlocks() const;
        const BallState& getBall() const;
        const PaddleState& getPaddle() const;
        const std::vector<BonusState>& getBonuses() const;
        const std::vector<ActiveEffectState>& getActiveEffects() const;

        void setScore(int s) { score = s; }
        void setLives(int l) { lives = l; }
        void setDifficulty(int d) { difficulty = d; }
        void setBlocks(const std::vector<BlockState>& b) { blocks = b; }
        void setBall(const BallState& b) { ballState = b; }
        void setPaddle(const PaddleState& p) { paddleState = p; }
        void setBonuses(const std::vector<BonusState>& b) { bonuses = b; }
        void setActiveEffects(const std::vector<ActiveEffectState>& a) { activeEffects = a; }
    };

    class SaveSystem {
    private:
        std::unique_ptr<GameMemento> save;
        std::string saveFile;

        void saveToFile();
        void loadFromFile();

    public:
        SaveSystem();
        ~SaveSystem();

        void saveGame(int score, int lives, int difficulty,
            const std::vector<std::unique_ptr<GameObject>>& blocks,
            const GameObject* ball,
            const GameObject* paddle,
            const std::vector<std::unique_ptr<Bonus>>& bonuses,
            const std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>>& activeEffects);

        bool loadGame();
        const GameMemento* getCurrentSave() const;
        bool hasSave() const;
        void clearSave();
    };

}