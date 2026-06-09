   #pragma once
// deklaracja zapowiadajaca:
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
    virtual ~Enemy() = default;

    virtual void update(double frameTime, Player& player, Map& map);
    virtual void takeDamage(int amount);
    bool isDead() const;
    int getDropItemTexture() const;

    double getX() const { return x; }
    double getY() const { return y; }
    int getTexture() const { return texture; }
    int getHp() const { return hp; }
    void setHp(int newHp) { hp = newHp; }
    AnimationState getState() const { return state; }
    bool isMirrored() const { return mirrored; }
    bool hasSpawnedDrop() const { return dropSpawned; }
    void setDropSpawned(bool value) { dropSpawned = value; }
    void setState(AnimationState a)  { state=a;}

protected:
    double x;
    double y;
    int texture;
    int hp;
    double attackTimer = 0.0; // odliczanie do nastepnego ataku
    int attackDamage = 15;    // ile obrazen zadaje ten przeciwnik
    AnimationState state = AnimationState::Idle;
    double hurtTimer = 0.0;
    double walkTimer = 0.0;
    bool mirrored = false;
    bool dropSpawned = false;
};


