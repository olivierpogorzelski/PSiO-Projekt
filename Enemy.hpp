#pragma once
// DEKLARACJA ZAPOWIADAJĄCA:
class Map;
class Player;
// klasa przeciwnika na mapie
class Enemy {
public:
    Enemy(double startX, double startY, int textureId);

    void update(double frameTime, Player& player, const Map& map);
    void takeDamage(int amount);
    bool isDead() const;

    double getX() const { return x; }
    double getY() const { return y; }
    int getTexture() const { return texture; }

private:
    double x;
    double y;
    int texture;
    int hp;
    double attackTimer = 0.0; // Odliczanie do następnego ataku
    int attackDamage = 15;    // Ile obrażeń zadaje ten przeciwnik
};
