#pragma once
#include "GameObject.hpp"

// deklaracje zapowiadające
class Player;
class Map;

class Projectile : public GameObject {
public:
    Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg, bool playerOwned = false);

    // zwraca true jeśli pocisk powinien zostać zniszczony (trafił w coś)
    bool update(double frameTime, Player& player, Map& map) override;

    int getTexture() const override { return texture; }

private:
    double dirX;
    double dirY;
    double moveSpeedPxPerSec;
    int texture;
    int damage;
    double hitboxRadius;
    bool isPlayerOwned;
};


