#include "ScoreSystem.h"
#include <fstream>
#include <algorithm>
#include <ctime>
#include <sstream>

namespace Arcanoid {

    ScoreSystem::ScoreSystem() : currentScore(0) {
        scoresFile = RESOURCES + "highscores.txt";
        loadHighScores();
    }

    ScoreSystem::~ScoreSystem() {
        saveHighScores();
    }

    void ScoreSystem::loadHighScores() {
        highScores.clear();
        std::ifstream file(scoresFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                int score;
                std::string date;
                std::string time;

                if (iss >> score >> date >> time) {
                    std::string fullDate = date + " " + time;
                    highScores.push_back(HighScore(score, fullDate));
                }
            }
            file.close();

            std::sort(highScores.begin(), highScores.end(),
                [](const HighScore& a, const HighScore& b) {
                    return a.score > b.score;
                });
        }
    }

    void ScoreSystem::saveHighScores() {
        std::ofstream file(scoresFile);
        if (file.is_open()) {
            int count = 0;
            for (const auto& hs : highScores) {
                if (count >= 10) break;
                file << hs.score << " " << hs.date << "\n";
                count++;
            }
            file.close();
        }
    }

    void ScoreSystem::addScore(int points) {
        currentScore += points;
    }

    void ScoreSystem::resetScore() {
        currentScore = 0;
    }

    int ScoreSystem::getCurrentScore() const {
        return currentScore;
    }

    void ScoreSystem::addHighScore(int score) {
        time_t now = time(nullptr);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &timeinfo);

        highScores.push_back(HighScore(score, std::string(buffer)));
        std::sort(highScores.begin(), highScores.end(),
            [](const HighScore& a, const HighScore& b) {
                return a.score > b.score;
            });
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
        saveHighScores();
    }

    const std::vector<HighScore>& ScoreSystem::getHighScores() const {
        return highScores;
    }

    bool ScoreSystem::isHighScore(int score) const {
        if (highScores.size() < 10) return true;
        return score > highScores.back().score;
    }

}