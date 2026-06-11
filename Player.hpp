#pragma once
#include "Map.hpp"

// klasa zarządzająca graczem
class Player {
public:
    Player(double startX, double startY, double startDirX, double startDirY, double startPlaneX, double startPlaneY);

    void update(double frameTime, Map& map);
    void attack(Map& map);
    void takeDamage(int amount);

    void addHp(int HP);
    double getX() const { return posX; }
    double getY() const { return posY; }
    double getDirX() const { return dirX; }
    double getDirY() const { return dirY; }
    double getPlaneX() const { return planeX; }
    double getPlaneY() const { return planeY; }
    int getHp() const { return hp; }
    int getScore() const { return score; }
    void addScore(int s) { score += s; }
    void setScore(int s) { score = s; }
    bool isDead() const { return hp <= 0; }
    
    // zwraca czas od ostatniego ataku do animacji miecza
    double getAttackTimer() const { return attackTimer; }
    
    // ekwipunek
    int inventoryWeapons[5]; // 0: puste, 1: miecz, 2: kusza
    int inventoryItems[5];   // 0: puste, 1: potka zdrowia
    int activeWeapon;        // zaznaczony slot (0 do 4)
    
    void setActiveWeaponSlot(int slot);
    int getActiveWeaponType() const; // zwraca typ broni (1 lub 2) z aktywnego slotu
    void usePotion();

    // settery dla systemu load/save
    void setX(double x) { posX = x; }
    void setY(double y) { posY = y; }
    void setDirX(double dx) { dirX = dx; }
    void setDirY(double dy) { dirY = dy; }
    void setPlaneX(double px) { planeX = px; }
    void setPlaneY(double py) { planeY = py; }
    void setHp(int h) { hp = h; }

private:
    double posX, posY;
    double dirX, dirY;
    double planeX, planeY;
    double attackTimer;
    int hp=100;
    int score = 0;
};


