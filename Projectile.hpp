#pragma once

// deklaracje zapowiadajace
class Player;
class Map;

class Projectile {
public:
    Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg, bool playerOwned = false);

    // zwraca true jesli pocisk powinien zostac zniszczony (trafil w cos)
    bool update(double frameTime, Player& player, Map& map);

    double getX() const { return x; }
    double getY() const { return y; }
    int getTexture() const { return texture; }

private:
    double x;
    double y;
    double dirX;
    double dirY;
    double speed;
    int texture;
    int damage;
    double hitboxRadius;
    bool isPlayerOwned;
};


