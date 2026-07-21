// GameState.h
#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include <vector>
#include "IGameObjectFactory.h"
#include "GameObjectFactory.h"
#include "Constants.h"
#include "ScoreSystem.h"
#include "SaveSystem.h"
#include "Bonus.h"

namespace Arcanoid {

    class GameState {
    private:
        enum State { MENU, PLAYING, PAUSED, GAMEOVER, VICTORY, SETTINGS, DIFFICULTY, HIGHSCORES };
        enum Difficulty { EASY, MEDIUM, HARD };

        struct DifficultySettings {
            int rows, cols;
            float paddleSpeed, ballSpeed;
        };

        std::unique_ptr<sf::RenderWindow> window;
        std::unique_ptr<sf::Font> font;
        std::unique_ptr<IGameObjectFactory> factory;

        std::unique_ptr<GameObject> paddle;
        std::unique_ptr<GameObject> ball;
        std::vector<std::unique_ptr<GameObject>> blocks;
        std::vector<std::unique_ptr<Bonus>> bonuses;

        ScoreSystem scoreSystem;
        SaveSystem saveSystem;

        sf::SoundBuffer bufferClick, bufferCrash, bufferBlockDestroy, bufferBonus;
        sf::Sound soundClick, soundCrash, soundBlockDestroy, soundBonus;
        sf::Music music;

        int score, lives;
        bool paused, victory;
        State currentState;
        Difficulty currentDifficulty;

        int menuSelection, difficultySelection, pauseSelection, gameOverSelection;
        bool soundEnabled, musicEnabled;

        std::vector<std::pair<std::unique_ptr<IBonusEffect>, float>> activeEffects;

        bool showSaveMessage;
        float saveMessageTimer;
        sf::Text saveMessageText;

        DifficultySettings getDifficultySettings() const;
        void createBlocks();
        void resetGame();
        void checkVictory();
        void checkBonusCollection();
        void updateBonuses(float dt);
        void updateActiveEffects(float dt);
        void updateSaveMessage(float dt);
        void resetAllEffects();
        void restoreBaseState(const GameMemento* memento);
        void restoreEffectsFromSave(const GameMemento* memento);
        void updateBlockVisuals();
        std::unique_ptr<IBonusEffect> createEffectFromState(const GameMemento::ActiveEffectState& state);
        void loadGameFromSave();

        void renderGame();
        void renderMenu();
        void renderPause();
        void renderGameOver();
        void renderVictory();
        void renderSettings();
        void renderDifficulty();
        void renderHighScores();

        void handleMenuInput(const sf::Event& event);
        void handlePauseInput(const sf::Event& event);
        void handleGameOverInput(const sf::Event& event);
        void handleVictoryInput(const sf::Event& event);
        void handleSettingsInput(const sf::Event& event);
        void handleDifficultyInput(const sf::Event& event);
        void handleHighScoresInput(const sf::Event& event);

        void showSaveMessageText(const std::string& message);

    public:
        GameState();
        ~GameState() = default;

        void update(float dt);
        void render();
        void handleInput(const sf::Event& event);
        bool isRunning() const;
        sf::RenderWindow& getWindow();
    };

}