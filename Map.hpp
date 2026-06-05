#pragma once
#include <vector>
#include "Enemy.hpp"
#include "Item.hpp"
#include "Projectile.hpp"
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
    std::vector<Item>& getItems();
    std::vector<Projectile>& getProjectiles();
    
    const std::vector<Item>& getItems() const;
    const std::vector<Enemy>& getEnemies() const;
    const std::vector<Projectile>& getProjectiles() const;

    void addProjectile(const Projectile& proj);

private:
    int worldMap[mapWidth][mapHeight];
    std::vector<Enemy> enemies;
    std::vector<Item> items;
    std::vector<Projectile> projectiles;
};
