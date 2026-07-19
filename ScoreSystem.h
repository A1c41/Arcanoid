#pragma once
#include <vector>
#include <string>
#include "Constants.h"

namespace Arcanoid {

    struct HighScore {
        int score;
        std::string date;

        HighScore() : score(0) {}
        HighScore(int s, const std::string& d) : score(s), date(d) {}
    };

    class ScoreSystem {
    private:
        int currentScore;
        std::vector<HighScore> highScores;
        std::string scoresFile;

        void loadHighScores();
        void saveHighScores();

    public:
        ScoreSystem();
        ~ScoreSystem();

        void addScore(int points);
        void resetScore();
        int getCurrentScore() const;
        void addHighScore(int score);
        const std::vector<HighScore>& getHighScores() const;
        bool isHighScore(int score) const;
    };

}