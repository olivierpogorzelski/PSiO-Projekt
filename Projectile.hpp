#pragma once

// deklaracje zapowiadające
class Player;
class Map;

class Projectile {
public:
    Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg);

    // zwraca true jeśli pocisk powinien zostać zniszczony (trafił w coś)
    bool update(double frameTime, Player& player, const Map& map);

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
};
