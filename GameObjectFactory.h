#pragma once
#include "IGameObjectFactory.h"
#include "Paddle.h"
#include "Ball.h"
#include "Block.h"
#include "StrongBlock.h"
#include "GlassBlock.h"
#include "IndestructibleBlock.h"

namespace Arcanoid {

    class GameObjectFactory : public IGameObjectFactory {
    public:
        std::unique_ptr<GameObject> createPaddle() override {
            return std::make_unique<Paddle>();
        }

        std::unique_ptr<GameObject> createBall() override {
            return std::make_unique<Ball>();
        }

        std::unique_ptr<GameObject> createBlock(float x, float y, sf::Color color) override {
            int type = rand() % 100;

            if (type < 15) {
                return std::make_unique<StrongBlock>(x, y, color, 2 + rand() % 3);
            }
            else if (type < 25) {
                return std::make_unique<GlassBlock>(x, y);
            }
            else if (type < 30) {
                return std::make_unique<IndestructibleBlock>(x, y);
            }
            else {
                return std::make_unique<Block>(x, y, color);
            }
        }
    };

}