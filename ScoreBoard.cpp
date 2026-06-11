#include "ScoreBoard.hpp"
#include <fstream>
#include <iostream>

ScoreBoard::ScoreBoard() : filename("highscores.txt") {
    loadScores();
}

void ScoreBoard::loadScores() {
    scores.clear();
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string name;
        int score;
        while (file >> name >> score) {
            scores.push_back({name, score});
        }
        file.close();
    }
}

void ScoreBoard::saveScores() {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& entry : scores) {
            file << entry.name << " " << entry.score << "\n";
        }
        file.close();
    }
}

void ScoreBoard::addScore(const std::string& name, int score) {
    scores.push_back({name, score});
    
    // sortuj malejąco
    std::sort(scores.begin(), scores.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.score > b.score;
    });
    
    // zostaw tylko top 10
    if (scores.size() > 10) {
        scores.resize(10);
    }
    
    saveScores();
}
