#pragma once
#include <vector>
#include "Enemy.hpp"
#include "Item.hpp"
#include "Projectile.hpp"
#include "Constants.hpp"
#include <memory>

class Player;

struct Door {
    int x, y;
    int state; // 0: zamknięte, 1: otwierają sie, 2: otwarte, 3: zamknięte
    double offset; // 0.0 (zamknięte) to 1.0 (otwarte)
    bool isVertical;
    double timer; // automatycznie zamyka po upłynięciu timera
};

/**
 * @class Map
 * @brief reprezentuje mapę gry, obsługuje kolizje ścian, drzwi oraz wszystkie encje (wrogowie, przedmioty, pociski).
 */
class Map {
public:
    static const int mapWidth = 40;
    static const int mapHeight = 40;

    Map();
    
    int getTile(int x, int y) const;
    void setTile(int x, int y, int value);
    bool isWalkable(int x, int y) const;
    int getWidth() const;
    int getHeight() const;

    void update(double frameTime, Player& player);
    
    void interactDoor(int x, int y);
    Door* getDoorAt(int x, int y);
    const Door* getDoorAt(int x, int y) const;
    const std::vector<Door>& getDoors() const;

    void clearEntities();
    void loadEnemy(double x, double y, int texture, int hp);
    void loadItem(double x, double y, int texture, bool pickedUp);



    void addProjectile(std::unique_ptr<Projectile> proj);

    // nowy wspólny kontener dla wszystkich obiektów gry
    std::vector<std::unique_ptr<GameObject>>& getEntities();
    const std::vector<std::unique_ptr<GameObject>>& getEntities() const;

private:
    int worldMap[mapWidth][mapHeight];
    std::vector<std::unique_ptr<GameObject>> entities;
    std::vector<std::unique_ptr<GameObject>> pendingEntities;
    std::vector<Door> doors;
};
