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
            float x, y;
            bool isActive;
            int hitsRemaining;
            bool isGlass;
            bool isIndestructible;
            int colorR, colorG, colorB;
        };

        struct BallState {
            float x, y;
            float vx, vy;
            float speed;
        };

        struct PaddleState {
            float x, y;
            float speed;
            float sizeX, sizeY;
        };

        struct BonusState {
            float x, y;
            int type;
            float remainingTime;
            bool isActive;
        };

        struct AffectedBlockInfo {
            int blockIndex;
            int originalHits;
        };

        struct ActiveEffectState {
            int type;
            float elapsedTime;
            float multiplier;
            std::vector<AffectedBlockInfo> affectedBlocks;
        };

    private:
        int score;
        int lives;
        std::vector<BlockState> blocks;
        BallState ballState;
        PaddleState paddleState;
        std::vector<BonusState> bonuses;
        std::vector<ActiveEffectState> activeEffects;

    public:
        GameMemento();

        void saveState(int score_, int lives_,
            const std::vector<std::unique_ptr<GameObject>>& blocks_,
            const GameObject* ball_,
            const GameObject* paddle_,
            const std::vector<std::unique_ptr<Bonus>>& bonuses_,
            const std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>>& activeEffects_);

        int getScore() const;
        int getLives() const;
        const std::vector<BlockState>& getBlocks() const;
        const BallState& getBall() const;
        const PaddleState& getPaddle() const;
        const std::vector<BonusState>& getBonuses() const;
        const std::vector<ActiveEffectState>& getActiveEffects() const;

        void setScore(int s) { score = s; }
        void setLives(int l) { lives = l; }
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

        void saveGame(int score, int lives,
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