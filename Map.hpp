#pragma once
#include <vector>
#include "Enemy.hpp"
#include "Constants.hpp"

// klasa mapy przechowująca strukturę korytarzy i wrogów
class Map {
public:
    Map();
    int getTile(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void update(double frameTime, Player& player);

    std::vector<Enemy>& getEnemies();
    const std::vector<Enemy>& getEnemies() const;

private:
    int worldMap[mapWidth][mapHeight];
    std::vector<Enemy> enemies;
};
