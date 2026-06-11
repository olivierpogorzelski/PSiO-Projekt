#pragma once
#include <string>
#include <vector>
#include <algorithm>

struct ScoreEntry {
    std::string name;
    int score;
};

class ScoreBoard {
public:
    ScoreBoard();
    
    void loadScores();
    void saveScores();
    void addScore(const std::string& name, int score);
    
    const std::vector<ScoreEntry>& getScores() const { return scores; }

private:
    std::vector<ScoreEntry> scores;
    std::string filename;
};
