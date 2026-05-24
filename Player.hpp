#pragma once
#include "Map.hpp"

// klasa zarządzająca graczem
class Player {
public:
    Player(double startX, double startY, double startDirX, double startDirY, double startPlaneX, double startPlaneY);

    void update(double frameTime, const Map& map);
    void attack(Map& map);

    double getX() const { return posX; }
    double getY() const { return posY; }
    double getDirX() const { return dirX; }
    double getDirY() const { return dirY; }
    double getPlaneX() const { return planeX; }
    double getPlaneY() const { return planeY; }
    
    // zwraca czas od ostatniego ataku do animacji miecza
    double getAttackTimer() const { return attackTimer; }

private:
    double posX, posY;
    double dirX, dirY;
    double planeX, planeY;
    double attackTimer;
};
