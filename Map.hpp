#pragma once
#include <vector>
#include "Enemy.hpp"
#include "Item.hpp"
#include "Projectile.hpp"
#include "Constants.hpp"
#include <memory>

// klasa mapy przechowujaca strukture korytarzy i wrogow
class Map {
public:
    Map();
    int getTile(int x, int y) const;
    int getWidth() const;
    int getHeight() const;
    void update(double frameTime, Player& player);

    // metody dla systemu zapisu/odczytu
    void clearEntities();
    void loadEnemy(double x, double y, int texture, int hp);
    void loadItem(double x, double y, int texture, bool pickedUp);

    std::vector<std::unique_ptr<Enemy>>& getEnemies();
    std::vector<Item>& getItems();
    std::vector<Projectile>& getProjectiles();
    
    const std::vector<Item>& getItems() const;
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const;
    const std::vector<Projectile>& getProjectiles() const;

    void addProjectile(const Projectile& proj);

private:
    int worldMap[mapWidth][mapHeight];
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Item> items;
    std::vector<Projectile> projectiles;
};


