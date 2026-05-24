#pragma once

// klasa przeciwnika na mapie
class Enemy {
public:
    Enemy(double startX, double startY, int textureId);

    void update(double frameTime);
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
};
