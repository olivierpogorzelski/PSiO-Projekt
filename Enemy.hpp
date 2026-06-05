#pragma once
// deklaracja zapowiadajĄca:
class Map;
class Player;

enum class AnimationState {
    Idle = 0,
    Walk = 1,
    Attack = 2,
    Hurt = 3,
    Dead = 4
};

// klasa przeciwnika na mapie
class Enemy {
public:
    Enemy(double startX, double startY, int textureE);

    void update(double frameTime, Player& player, Map& map);
    void takeDamage(int amount);
    bool isDead() const;

    double getX() const { return x; }
    double getY() const { return y; }
    int getTexture() const { return texture; }
    AnimationState getState() const { return state; }
    bool isMirrored() const { return mirrored; }

private:
    double x;
    double y;
    int texture;
    int hp;
    double attackTimer = 0.0; // odliczanie do następnego ataku
    int attackDamage = 15;    // ile obražeń zadaje ten przeciwnik
    AnimationState state = AnimationState::Idle;
    double hurtTimer = 0.0;
    double walkTimer = 0.0;
    bool mirrored = false;
};
