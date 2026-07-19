#pragma once
#include <memory>
#include <SFML/Graphics.hpp>
#include "GameObject.h"

namespace Arcanoid {

    class IGameObjectFactory {
    public:
        virtual ~IGameObjectFactory() = default;

        virtual std::unique_ptr<GameObject> createPaddle() = 0;
        virtual std::unique_ptr<GameObject> createBall() = 0;
        virtual std::unique_ptr<GameObject> createBlock(float x, float y, sf::Color color) = 0;
    };

}