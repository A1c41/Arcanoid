// GameState.cpp
#include "GameState.h"
#include "Paddle.h"
#include "Ball.h"
#include "Block.h"
#include "StrongBlock.h"
#include "GlassBlock.h"
#include "IndestructibleBlock.h"
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace Arcanoid {

    GameState::GameState() : score(0), lives(3),
        paused(false), victory(false),
        currentState(MENU), currentDifficulty(MEDIUM),
        menuSelection(0), difficultySelection(0), pauseSelection(0),
        gameOverSelection(0), soundEnabled(true), musicEnabled(true),
        showSaveMessage(false), saveMessageTimer(0.0f) {

        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Arcanoid");
        window->setFramerateLimit(60);

        font = std::make_unique<sf::Font>();
        if (!font->loadFromFile(RESOURCES + "fonts\\Roboto-Black.ttf")) {
            font->loadFromFile("C:/Windows/Fonts/arial.ttf");
        }

        factory = std::make_unique<GameObjectFactory>();
        paddle = factory->createPaddle();
        ball = factory->createBall();

        bufferClick.loadFromFile(RESOURCES + "click.wav");
        bufferCrash.loadFromFile(RESOURCES + "Death.wav");
        bufferBlockDestroy.loadFromFile(RESOURCES + "block_destroy.wav");
        bufferBonus.loadFromFile(RESOURCES + "bonus.wav");
        soundClick.setBuffer(bufferClick);
        soundCrash.setBuffer(bufferCrash);
        soundBlockDestroy.setBuffer(bufferBlockDestroy);
        soundBonus.setBuffer(bufferBonus);

        if (music.openFromFile(RESOURCES + "music.wav")) {
            music.setLoop(true);
            music.setVolume(50);
        }

        saveMessageText.setFont(*font);
        saveMessageText.setCharacterSize(36);
        saveMessageText.setFillColor(sf::Color::Green);
        saveMessageText.setPosition(WINDOW_WIDTH / 2.0f - 100.0f, WINDOW_HEIGHT / 2.0f - 50.0f);

        createBlocks();
    }

    GameState::DifficultySettings GameState::getDifficultySettings() const {
        switch (currentDifficulty) {
        case EASY:   return { 3, 6, 300.0f, 350.0f };
        case MEDIUM: return { 5, 8, 350.0f, 400.0f };
        case HARD:   return { 7, 10, 450.0f, 500.0f };
        default:     return { 5, 8, 350.0f, 400.0f };
        }
    }

    void GameState::createBlocks() {
        blocks.clear();
        auto settings = getDifficultySettings();

        float w = 80.0f, h = 30.0f, spacing = 8.0f;
        float totalW = settings.cols * (w + spacing) - spacing;
        float startX = (WINDOW_WIDTH - totalW) / 2.0f;
        float startY = 60.0f;

        std::vector<sf::Color> colors = {
            {128, 0, 128}, {0, 128, 0}, {255, 0, 0},
            {0, 0, 255}, {255, 165, 0}, {255, 255, 0}, {0, 255, 255}
        };

        for (int row = 0; row < settings.rows; ++row) {
            sf::Color rowColor = colors[row % colors.size()];
            for (int col = 0; col < settings.cols; ++col) {
                float x = startX + col * (w + spacing);
                float y = startY + row * (h + spacing);

                auto block = factory->createBlock(x, y, rowColor);
                blocks.push_back(std::move(block));
            }
        }
    }

    void GameState::resetAllEffects() {
        Paddle* paddlePtr = static_cast<Paddle*>(paddle.get());
        Ball* ballPtr = static_cast<Ball*>(ball.get());
        for (auto& effect : activeEffects) {
            effect.first->remove(paddlePtr, ballPtr, blocks);
            effect.first->reset();
        }
        activeEffects.clear();
    }

    void GameState::resetGame() {
        auto settings = getDifficultySettings();

        resetAllEffects();

        paddle->setPosition(WINDOW_WIDTH / 2.0f - 60.0f, WINDOW_HEIGHT - 50.0f);
        static_cast<Paddle*>(paddle.get())->setSpeed(settings.paddleSpeed);

        auto* ballPtr = static_cast<Ball*>(ball.get());
        ballPtr->setSpeed(settings.ballSpeed);
        ballPtr->reset();

        createBlocks();
        scoreSystem.resetScore();
        score = 0;
        lives = 3;
        victory = false;
        paused = false;
        bonuses.clear();
        currentState = PLAYING;

        saveSystem.clearSave();

        if (musicEnabled && music.getStatus() != sf::Music::Playing) {
            music.play();
        }
    }

    void GameState::restoreBaseState(const GameMemento* memento) {
        lives = memento->getLives();
        score = memento->getScore();
        scoreSystem.resetScore();
        scoreSystem.addScore(score);

        auto* ballPtr = static_cast<Ball*>(ball.get());
        const auto& ballState = memento->getBall();
        ballPtr->setPosition(ballState.x, ballState.y);
        ballPtr->setVelocity({ ballState.vx, ballState.vy });
        ballPtr->setSpeed(ballState.speed);

        const auto& paddleState = memento->getPaddle();
        paddle->setPosition(paddleState.x, paddleState.y);
        static_cast<Paddle*>(paddle.get())->setSpeed(paddleState.speed);
        static_cast<Paddle*>(paddle.get())->setSize(sf::Vector2f(paddleState.sizeX, paddleState.sizeY));

        blocks.clear();
        const auto& blockStates = memento->getBlocks();
        for (const auto& state : blockStates) {
            sf::Color color(state.colorR, state.colorG, state.colorB);
            std::unique_ptr<GameObject> block;

            switch (state.type) {
            case GameMemento::BlockState::INDESTRUCTIBLE:
                block = std::make_unique<IndestructibleBlock>(state.x, state.y);
                break;
            case GameMemento::BlockState::GLASS:
                block = std::make_unique<GlassBlock>(state.x, state.y);
                if (!state.isActive) {
                    static_cast<GlassBlock*>(block.get())->destroy();
                }
                break;
            case GameMemento::BlockState::STRONG:
                block = std::make_unique<StrongBlock>(state.x, state.y, color, state.maxHits);
                if (state.isActive) {
                    static_cast<StrongBlock*>(block.get())->setHitsRemaining(state.hitsRemaining);
                    static_cast<StrongBlock*>(block.get())->updateAppearance();
                }
                else {
                    static_cast<StrongBlock*>(block.get())->setHitsRemaining(0);
                }
                break;
            default:
                block = std::make_unique<Block>(state.x, state.y, color);
                if (!state.isActive) {
                    static_cast<Block*>(block.get())->destroy();
                }
                break;
            }
            blocks.push_back(std::move(block));
        }

        bonuses.clear();
        const auto& bonusStates = memento->getBonuses();
        for (const auto& bonusState : bonusStates) {
            if (bonusState.isActive) {
                auto newBonus = std::make_unique<Bonus>(
                    bonusState.x, bonusState.y,
                    static_cast<Bonus::Type>(bonusState.type)
                );
                bonuses.push_back(std::move(newBonus));
            }
        }
    }

    std::unique_ptr<IBonusEffect> GameState::createEffectFromState(const GameMemento::ActiveEffectState& state) {
        std::unique_ptr<IBonusEffect> effect;
        switch (state.type) {
        case 0: effect = std::make_unique<FireBallEffect>(); break;
        case 1: effect = std::make_unique<FragileBlocksEffect>(); break;
        case 2: effect = std::make_unique<PaddleSizeEffect>(1.5f); break;
        case 3: effect = std::make_unique<PaddleSizeEffect>(0.7f); break;
        case 4: effect = std::make_unique<PaddleSpeedEffect>(1.5f); break;
        case 5: effect = std::make_unique<PaddleSpeedEffect>(0.7f); break;
        }
        return effect;
    }

    void GameState::checkVictory() {
        int activeDestroyableBlocks = 0;
        for (auto& block : blocks) {
            auto* blockPtr = static_cast<Block*>(block.get());
            if (dynamic_cast<IndestructibleBlock*>(block.get()) == nullptr && blockPtr->isAlive()) {
                activeDestroyableBlocks++;
            }
        }

        if (activeDestroyableBlocks == 0) {
            victory = true;
            currentState = VICTORY;
            if (musicEnabled) music.pause();

            if (scoreSystem.isHighScore(score)) {
                scoreSystem.addHighScore(score);
            }
        }
    }

    void GameState::checkBonusCollection() {
        for (auto it = bonuses.begin(); it != bonuses.end();) {
            if (!(*it)->isAlive()) {
                it = bonuses.erase(it);
                continue;
            }

            if ((*it)->getBounds().intersects(paddle->getBounds())) {
                auto* bonus = it->get();
                auto effect = bonus->releaseEffect();

                if (effect) {
                    Paddle* paddlePtr = static_cast<Paddle*>(paddle.get());
                    Ball* ballPtr = static_cast<Ball*>(ball.get());

                    effect->apply(paddlePtr, ballPtr, blocks);
                    activeEffects.push_back({ std::move(effect), 0.0f });

                    if (soundEnabled) soundBonus.play();
                }

                bonus->collect();
                it = bonuses.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void GameState::updateBonuses(float dt) {
        for (auto& bonus : bonuses) {
            bonus->update(dt);
        }
    }

    void GameState::updateSaveMessage(float dt) {
        if (showSaveMessage) {
            saveMessageTimer -= dt;
            if (saveMessageTimer <= 0.0f) {
                showSaveMessage = false;
            }
        }
    }

    void GameState::showSaveMessageText(const std::string& message) {
        saveMessageText.setString(message);
        sf::FloatRect textBounds = saveMessageText.getLocalBounds();
        saveMessageText.setPosition(
            WINDOW_WIDTH / 2.0f - textBounds.width / 2.0f,
            WINDOW_HEIGHT / 2.0f - textBounds.height / 2.0f
        );
        showSaveMessage = true;
        saveMessageTimer = 2.0f;
    }

    void GameState::updateActiveEffects(float dt) {
        for (auto it = activeEffects.begin(); it != activeEffects.end();) {
            it->second += dt;
            if (it->second >= it->first->getDuration()) {
                Paddle* paddlePtr = static_cast<Paddle*>(paddle.get());
                Ball* ballPtr = static_cast<Ball*>(ball.get());

                it->first->remove(paddlePtr, ballPtr, blocks);
                it = activeEffects.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void GameState::loadGameFromSave() {
        saveSystem.loadGame();
        auto* memento = saveSystem.getCurrentSave();
        if (!memento) return;

        resetAllEffects();

        currentDifficulty = static_cast<Difficulty>(memento->getDifficulty());

        restoreBaseState(memento);

        const auto& effectStates = memento->getActiveEffects();
        for (const auto& effectState : effectStates) {
            auto effect = createEffectFromState(effectState);
            if (effect) {
                effect->restoreState(effectState.elapsedTime);
                Paddle* paddlePtr = static_cast<Paddle*>(paddle.get());
                Ball* ballPtr = static_cast<Ball*>(ball.get());
                effect->apply(paddlePtr, ballPtr, blocks);
                activeEffects.push_back({ std::move(effect), effectState.elapsedTime });
            }
        }

        saveSystem.clearSave();
    }

    void GameState::update(float dt) {
        updateSaveMessage(dt);

        if (currentState != PLAYING || paused) return;

        paddle->update(dt);
        ball->update(dt);
        updateBonuses(dt);
        updateActiveEffects(dt);
        checkBonusCollection();

        auto* ballPtr = static_cast<Ball*>(ball.get());

        if (ball->getBounds().intersects(paddle->getBounds())) {
            ballPtr->bounceFromPaddle(*paddle);
            if (soundEnabled) soundClick.play();
        }

        bool isFireBall = false;
        for (const auto& effect : activeEffects) {
            if (auto* fireEffect = dynamic_cast<FireBallEffect*>(effect.first.get())) {
                if (fireEffect->isFireBallActive()) {
                    isFireBall = true;
                    break;
                }
            }
        }

        for (auto& block : blocks) {
            if (!static_cast<Block*>(block.get())->isAlive()) continue;

            if (ball->getBounds().intersects(block->getBounds())) {
                bool isGlass = dynamic_cast<GlassBlock*>(block.get()) != nullptr;
                bool isStrong = dynamic_cast<StrongBlock*>(block.get()) != nullptr;
                bool isIndestructible = dynamic_cast<IndestructibleBlock*>(block.get()) != nullptr;

                if (isIndestructible) {
                    if (!isFireBall) {
                        sf::FloatRect bb = ball->getBounds();
                        sf::FloatRect bk = block->getBounds();
                        float overlapX = (bb.left + bb.width / 2) - (bk.left + bk.width / 2);
                        float overlapY = (bb.top + bb.height / 2) - (bk.top + bk.height / 2);

                        if (std::abs(overlapX / bk.width) > std::abs(overlapY / bk.height)) {
                            ballPtr->reflectX();
                        }
                        else {
                            ballPtr->reflectY();
                        }
                    }
                    break;
                }

                int pointsEarned = 0;
                bool blockDestroyed = false;

                if (isStrong) {
                    auto* strongBlock = static_cast<StrongBlock*>(block.get());
                    strongBlock->hit();

                    if (!strongBlock->isAlive()) {
                        blockDestroyed = true;
                        pointsEarned = 10;
                        if (soundEnabled) soundBlockDestroy.play();
                    }

                    if (!isFireBall) {
                        sf::FloatRect bb = ball->getBounds();
                        sf::FloatRect bk = block->getBounds();
                        float overlapX = (bb.left + bb.width / 2) - (bk.left + bk.width / 2);
                        float overlapY = (bb.top + bb.height / 2) - (bk.top + bk.height / 2);

                        if (std::abs(overlapX / bk.width) > std::abs(overlapY / bk.height)) {
                            ballPtr->reflectX();
                        }
                        else {
                            ballPtr->reflectY();
                        }
                    }
                }
                else if (isGlass) {
                    if (static_cast<GlassBlock*>(block.get())->isAlive()) {
                        static_cast<GlassBlock*>(block.get())->destroy();
                        blockDestroyed = true;
                        pointsEarned = 15;
                        if (soundEnabled) soundBlockDestroy.play();
                    }
                }
                else {
                    if (static_cast<Block*>(block.get())->isAlive()) {
                        static_cast<Block*>(block.get())->destroy();
                        blockDestroyed = true;
                        pointsEarned = 10;
                        if (soundEnabled) soundBlockDestroy.play();
                    }

                    if (!isFireBall) {
                        sf::FloatRect bb = ball->getBounds();
                        sf::FloatRect bk = block->getBounds();
                        float overlapX = (bb.left + bb.width / 2) - (bk.left + bk.width / 2);
                        float overlapY = (bb.top + bb.height / 2) - (bk.top + bk.height / 2);

                        if (std::abs(overlapX / bk.width) > std::abs(overlapY / bk.height)) {
                            ballPtr->reflectX();
                        }
                        else {
                            ballPtr->reflectY();
                        }
                    }
                }

                if (blockDestroyed) {
                    scoreSystem.addScore(pointsEarned);
                    score = scoreSystem.getCurrentScore();

                    if (rand() % 100 < 10) {
                        sf::FloatRect bounds = block->getBounds();
                        auto bonus = BonusFactory::createRandomBonus(
                            bounds.left + bounds.width / 2 - 10,
                            bounds.top + bounds.height / 2 - 10
                        );
                        bonuses.push_back(std::move(bonus));
                    }
                }

                checkVictory();
                break;
            }
        }

        if (ballPtr->isOutOfBounds()) {
            if (saveSystem.hasSave()) {
                loadGameFromSave();
                if (soundEnabled) soundCrash.play();
            }
            else {
                lives--;
                if (lives <= 0) {
                    currentState = GAMEOVER;
                    if (soundEnabled) soundCrash.play();
                    if (musicEnabled) music.pause();

                    if (scoreSystem.isHighScore(score)) {
                        scoreSystem.addHighScore(score);
                    }
                }
                else {
                    ballPtr->reset();
                    if (soundEnabled) soundCrash.play();
                }
            }
        }
    }

    void GameState::render() {
        window->clear(sf::Color(20, 20, 40));
        switch (currentState) {
        case MENU: renderMenu(); break;
        case PLAYING: renderGame(); break;
        case PAUSED: renderPause(); break;
        case GAMEOVER: renderGameOver(); break;
        case VICTORY: renderVictory(); break;
        case SETTINGS: renderSettings(); break;
        case DIFFICULTY: renderDifficulty(); break;
        case HIGHSCORES: renderHighScores(); break;
        }

        if (showSaveMessage) {
            sf::FloatRect textBounds = saveMessageText.getLocalBounds();
            sf::RectangleShape background({ textBounds.width + 40.0f, textBounds.height + 20.0f });
            background.setFillColor(sf::Color(0, 0, 0, 150));
            background.setPosition(
                WINDOW_WIDTH / 2.0f - textBounds.width / 2.0f - 20.0f,
                WINDOW_HEIGHT / 2.0f - textBounds.height / 2.0f - 10.0f
            );
            background.setOutlineColor(sf::Color::Green);
            background.setOutlineThickness(2.0f);
            window->draw(background);

            window->draw(saveMessageText);
        }

        window->display();
    }

    void GameState::renderGame() {
        paddle->draw(*window);
        ball->draw(*window);
        for (auto& block : blocks) block->draw(*window);
        for (auto& bonus : bonuses) bonus->draw(*window);

        int destroyableTotal = 0;
        int destroyableDestroyed = 0;
        for (auto& block : blocks) {
            auto* blockPtr = static_cast<Block*>(block.get());
            if (dynamic_cast<IndestructibleBlock*>(block.get()) != nullptr) {
                continue;
            }
            destroyableTotal++;
            if (!blockPtr->isAlive()) {
                destroyableDestroyed++;
            }
        }

        sf::Text text;
        text.setFont(*font);
        text.setString("Score: " + std::to_string(score) + "  Lives: " + std::to_string(lives) +
            "  Blocks: " + std::to_string(destroyableDestroyed) + "/" + std::to_string(destroyableTotal));
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        text.setPosition(15.0f, 15.0f);
        window->draw(text);

        float yOffset = 60.0f;
        for (const auto& effect : activeEffects) {
            sf::Text bonusText;
            bonusText.setFont(*font);
            float remaining = effect.first->getDuration() - effect.second;
            bonusText.setString("Bonus: " + std::to_string((int)remaining) + "s");
            bonusText.setCharacterSize(18);
            bonusText.setFillColor(effect.first->getColor());
            bonusText.setPosition(15.0f, yOffset);
            window->draw(bonusText);
            yOffset += 25.0f;
        }

        sf::Text pauseHint;
        pauseHint.setFont(*font);
        pauseHint.setString("P: Pause  S: Save");
        pauseHint.setCharacterSize(18);
        pauseHint.setFillColor(sf::Color(200, 200, 200));
        pauseHint.setPosition(WINDOW_WIDTH - 200.0f, 15.0f);
        window->draw(pauseHint);
    }

    void GameState::renderMenu() {
        sf::Text title("ARCANOID", *font, 72);
        title.setFillColor(sf::Color::White);
        title.setPosition(WINDOW_WIDTH / 2.0f - 150.0f, 80.0f);
        window->draw(title);

        std::vector<std::string> items = { "Play", "Difficulty", "High Scores", "Settings", "Exit" };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 34);
            item.setFillColor(i == menuSelection ? sf::Color::Yellow : sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 80.0f, 200.0f + i * 60.0f);
            window->draw(item);
        }

        std::string diffText = "Current: ";
        switch (currentDifficulty) {
        case EASY: diffText += "Easy"; break;
        case MEDIUM: diffText += "Medium"; break;
        case HARD: diffText += "Hard"; break;
        }
        sf::Text diffInfo(diffText, *font, 22);
        diffInfo.setFillColor(sf::Color(200, 200, 200));
        diffInfo.setPosition(WINDOW_WIDTH / 2.0f - 70.0f, 500.0f);
        window->draw(diffInfo);
    }

    void GameState::renderPause() {
        renderGame();
        sf::RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
        overlay.setFillColor({ 0, 0, 0, 180 });
        window->draw(overlay);

        sf::Text title("PAUSED", *font, 56);
        title.setFillColor(sf::Color::White);
        title.setPosition(WINDOW_WIDTH / 2.0f - 100.0f, 150.0f);
        window->draw(title);

        std::vector<std::string> items = { "Resume", "Save Game", "Main Menu" };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 32);
            item.setFillColor(i == pauseSelection ? sf::Color::Yellow : sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 80.0f, 280.0f + i * 60.0f);
            window->draw(item);
        }
    }

    void GameState::renderGameOver() {
        renderGame();
        sf::RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
        overlay.setFillColor({ 0, 0, 0, 180 });
        window->draw(overlay);

        sf::Text text("GAME OVER", *font, 72);
        text.setFillColor(sf::Color::Red);
        text.setPosition(WINDOW_WIDTH / 2.0f - 160.0f, WINDOW_HEIGHT / 2.0f - 150.0f);
        window->draw(text);

        sf::Text scoreText("Final Score: " + std::to_string(score), *font, 34);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(WINDOW_WIDTH / 2.0f - 100.0f, WINDOW_HEIGHT / 2.0f - 60.0f);
        window->draw(scoreText);

        if (scoreSystem.isHighScore(score)) {
            sf::Text highScoreText("NEW HIGH SCORE!", *font, 28);
            highScoreText.setFillColor(sf::Color::Yellow);
            highScoreText.setPosition(WINDOW_WIDTH / 2.0f - 130.0f, WINDOW_HEIGHT / 2.0f - 20.0f);
            window->draw(highScoreText);
        }

        std::vector<std::string> items = { "Play Again", "High Scores", "Main Menu" };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 28);
            item.setFillColor(i == gameOverSelection ? sf::Color::Yellow : sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 80.0f, WINDOW_HEIGHT / 2.0f + 40.0f + i * 50.0f);
            window->draw(item);
        }
    }

    void GameState::renderVictory() {
        renderGame();
        sf::RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
        overlay.setFillColor({ 0, 0, 0, 180 });
        window->draw(overlay);

        sf::Text text("VICTORY!", *font, 80);
        text.setFillColor(sf::Color::Yellow);
        text.setPosition(WINDOW_WIDTH / 2.0f - 170.0f, WINDOW_HEIGHT / 2.0f - 180.0f);
        window->draw(text);

        sf::Text congratsText("Congratulations! You destroyed all blocks!", *font, 32);
        congratsText.setFillColor(sf::Color::White);
        congratsText.setPosition(WINDOW_WIDTH / 2.0f - 250.0f, WINDOW_HEIGHT / 2.0f - 100.0f);
        window->draw(congratsText);

        sf::Text scoreText("Final Score: " + std::to_string(score), *font, 34);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(WINDOW_WIDTH / 2.0f - 100.0f, WINDOW_HEIGHT / 2.0f - 40.0f);
        window->draw(scoreText);

        if (scoreSystem.isHighScore(score)) {
            sf::Text highScoreText("NEW HIGH SCORE!", *font, 28);
            highScoreText.setFillColor(sf::Color::Yellow);
            highScoreText.setPosition(WINDOW_WIDTH / 2.0f - 130.0f, WINDOW_HEIGHT / 2.0f);
            window->draw(highScoreText);
        }

        std::vector<std::string> items = { "Play Again", "High Scores", "Main Menu" };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 28);
            item.setFillColor(i == gameOverSelection ? sf::Color::Yellow : sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 80.0f, WINDOW_HEIGHT / 2.0f + 60.0f + i * 50.0f);
            window->draw(item);
        }
    }

    void GameState::renderSettings() {
        sf::Text title("SETTINGS", *font, 56);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(WINDOW_WIDTH / 2.0f - 130.0f, 100.0f);
        window->draw(title);

        std::vector<std::string> items = {
            "Sound: " + std::string(soundEnabled ? "ON" : "OFF"),
            "Music: " + std::string(musicEnabled ? "ON" : "OFF")
        };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 34);
            item.setFillColor(i == menuSelection ? sf::Color::Yellow : sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 120.0f, 250.0f + i * 70.0f);
            window->draw(item);
        }

        sf::Text hint("Use Up/Down to select, Left/Right to toggle", *font, 20);
        hint.setFillColor(sf::Color(200, 200, 200));
        hint.setPosition(WINDOW_WIDTH / 2.0f - 200.0f, WINDOW_HEIGHT - 70.0f);
        window->draw(hint);

        sf::Text backHint("Press Escape to return to menu", *font, 20);
        backHint.setFillColor(sf::Color(200, 200, 200));
        backHint.setPosition(WINDOW_WIDTH / 2.0f - 160.0f, WINDOW_HEIGHT - 40.0f);
        window->draw(backHint);
    }

    void GameState::renderDifficulty() {
        sf::Text title("DIFFICULTY", *font, 56);
        title.setFillColor(sf::Color::Cyan);
        title.setPosition(WINDOW_WIDTH / 2.0f - 150.0f, 100.0f);
        window->draw(title);

        std::vector<std::string> items = { "Easy", "Medium", "Hard" };
        for (size_t i = 0; i < items.size(); ++i) {
            sf::Text item(items[i], *font, 34);
            bool selected = (i == difficultySelection);
            bool current = (i == static_cast<int>(currentDifficulty));
            if (selected) item.setFillColor(sf::Color::Yellow);
            else if (current) item.setFillColor(sf::Color::Green);
            else item.setFillColor(sf::Color::White);
            item.setPosition(WINDOW_WIDTH / 2.0f - 70.0f, 250.0f + i * 70.0f);
            window->draw(item);

            if (current) {
                sf::Text currentText("(current)", *font, 20);
                currentText.setFillColor(sf::Color::Green);
                currentText.setPosition(WINDOW_WIDTH / 2.0f + 100.0f, 255.0f + i * 70.0f);
                window->draw(currentText);
            }
        }

        sf::Text hint("Use Up/Down to select, Enter to confirm, Escape to return", *font, 20);
        hint.setFillColor(sf::Color(200, 200, 200));
        hint.setPosition(WINDOW_WIDTH / 2.0f - 260.0f, WINDOW_HEIGHT - 60.0f);
        window->draw(hint);
    }

    void GameState::renderHighScores() {
        sf::Text title("HIGH SCORES", *font, 56);
        title.setFillColor(sf::Color::Yellow);
        title.setPosition(WINDOW_WIDTH / 2.0f - 160.0f, 80.0f);
        window->draw(title);

        const auto& scores = scoreSystem.getHighScores();
        if (scores.empty()) {
            sf::Text emptyText("No high scores yet!", *font, 30);
            emptyText.setFillColor(sf::Color::White);
            emptyText.setPosition(WINDOW_WIDTH / 2.0f - 130.0f, 250.0f);
            window->draw(emptyText);
        }
        else {
            float yPos = 180.0f;
            for (size_t i = 0; i < scores.size() && i < 10; ++i) {
                std::stringstream ss;
                ss << "#" << (i + 1) << "  " << scores[i].score << "  " << scores[i].date;
                sf::Text scoreText(ss.str(), *font, 26);
                scoreText.setFillColor(i == 0 ? sf::Color::Yellow : sf::Color::White);
                scoreText.setPosition(WINDOW_WIDTH / 2.0f - 150.0f, yPos);
                window->draw(scoreText);
                yPos += 40.0f;
            }
        }

        sf::Text backHint("Press Escape to return to menu", *font, 20);
        backHint.setFillColor(sf::Color(200, 200, 200));
        backHint.setPosition(WINDOW_WIDTH / 2.0f - 160.0f, WINDOW_HEIGHT - 40.0f);
        window->draw(backHint);
    }

    void GameState::handleInput(const sf::Event& event) {
        if (event.type == sf::Event::Closed) {
            window->close();
            return;
        }

        if (currentState == PLAYING && !paused) {
            paddle->handleInput(event);
        }

        if (event.type != sf::Event::KeyPressed) {
            return;
        }

        switch (currentState) {
        case MENU: handleMenuInput(event); break;
        case PLAYING: {
            if (event.key.code == sf::Keyboard::P) {
                paused = !paused;
                currentState = paused ? PAUSED : PLAYING;
                if (paused && musicEnabled) music.pause();
                else if (!paused && musicEnabled && music.getStatus() != sf::Music::Playing) music.play();
            }
            if (event.key.code == sf::Keyboard::S && !paused) {
                saveSystem.saveGame(score, lives, static_cast<int>(currentDifficulty),
                    blocks, ball.get(), paddle.get(), bonuses, activeEffects);
                showSaveMessageText("Game Saved!");
            }
            break;
        }
        case PAUSED: handlePauseInput(event); break;
        case GAMEOVER: handleGameOverInput(event); break;
        case VICTORY: handleVictoryInput(event); break;
        case SETTINGS: handleSettingsInput(event); break;
        case DIFFICULTY: handleDifficultyInput(event); break;
        case HIGHSCORES: handleHighScoresInput(event); break;
        }
    }

    void GameState::handleMenuInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            menuSelection = (menuSelection - 1 + 5) % 5;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            menuSelection = (menuSelection + 1) % 5;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            switch (menuSelection) {
            case 0: resetGame(); break;
            case 1: currentState = DIFFICULTY; difficultySelection = static_cast<int>(currentDifficulty); break;
            case 2: currentState = HIGHSCORES; break;
            case 3: currentState = SETTINGS; menuSelection = 0; break;
            case 4: window->close(); break;
            }
        }
    }

    void GameState::handlePauseInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            pauseSelection = (pauseSelection - 1 + 3) % 3;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            pauseSelection = (pauseSelection + 1) % 3;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (pauseSelection == 0) {
                paused = false;
                currentState = PLAYING;
                if (musicEnabled && music.getStatus() != sf::Music::Playing) music.play();
            }
            else if (pauseSelection == 1) {
                saveSystem.saveGame(score, lives, static_cast<int>(currentDifficulty),
                    blocks, ball.get(), paddle.get(), bonuses, activeEffects);
                showSaveMessageText("Game Saved!");
            }
            else {
                paused = false;
                currentState = MENU;
                menuSelection = 0;
                if (musicEnabled) music.pause();
            }
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            paused = false;
            currentState = PLAYING;
            if (musicEnabled && music.getStatus() != sf::Music::Playing) music.play();
        }
    }

    void GameState::handleGameOverInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            gameOverSelection = (gameOverSelection - 1 + 3) % 3;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            gameOverSelection = (gameOverSelection + 1) % 3;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (gameOverSelection == 0) {
                resetGame();
            }
            else if (gameOverSelection == 1) {
                currentState = HIGHSCORES;
            }
            else {
                currentState = MENU;
                menuSelection = 0;
                if (musicEnabled) music.pause();
            }
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            currentState = MENU;
            menuSelection = 0;
            if (musicEnabled) music.pause();
        }
    }

    void GameState::handleVictoryInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            gameOverSelection = (gameOverSelection - 1 + 3) % 3;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            gameOverSelection = (gameOverSelection + 1) % 3;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            if (gameOverSelection == 0) {
                resetGame();
            }
            else if (gameOverSelection == 1) {
                currentState = HIGHSCORES;
            }
            else {
                currentState = MENU;
                menuSelection = 0;
                if (musicEnabled) music.pause();
            }
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            currentState = MENU;
            menuSelection = 0;
            if (musicEnabled) music.pause();
        }
    }

    void GameState::handleSettingsInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            menuSelection = (menuSelection - 1 + 2) % 2;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            menuSelection = (menuSelection + 1) % 2;
        }
        else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right) {
            if (menuSelection == 0) {
                soundEnabled = !soundEnabled;
            }
            else {
                musicEnabled = !musicEnabled;
                if (musicEnabled) music.setVolume(50);
                else music.setVolume(0);
            }
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            currentState = MENU;
            menuSelection = 0;
            if (musicEnabled) music.setVolume(50);
        }
    }

    void GameState::handleDifficultyInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            difficultySelection = (difficultySelection - 1 + 3) % 3;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            difficultySelection = (difficultySelection + 1) % 3;
        }
        else if (event.key.code == sf::Keyboard::Enter) {
            currentDifficulty = static_cast<Difficulty>(difficultySelection);
            currentState = MENU;
            menuSelection = 0;
        }
        else if (event.key.code == sf::Keyboard::Escape) {
            currentState = MENU;
            menuSelection = 0;
        }
    }

    void GameState::handleHighScoresInput(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::Escape) {
            currentState = MENU;
            menuSelection = 0;
        }
    }

    bool GameState::isRunning() const {
        return window->isOpen();
    }

    sf::RenderWindow& GameState::getWindow() {
        return *window;
    }

}