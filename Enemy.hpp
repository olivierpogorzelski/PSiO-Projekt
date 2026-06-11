#pragma once
#include "GameObject.hpp"
#include <string>

// deklaracja zapowiadająca:
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
class Enemy : public GameObject {
public:
    Enemy(double startX, double startY, int textureE);
    virtual ~Enemy() = default;

    bool update(double frameTime, Player& player, Map& map) override;
    virtual void takeDamage(int amount);
    bool isDead() const;

    int getTexture() const override { return texture; }
    int getHp() const { return hp; }
    void setHp(int newHp) { hp = newHp; }
    void setAttackTimer(double t) { attackTimer = t; }
    int getState() const override { return static_cast<int>(state); }
    bool isMirrored() const override { return mirrored; }
    std::string getSoundPrefix() const;

protected:
    int texture;
    int hp;
    double attackTimer = 0.0; // odliczanie do następnego ataku
    int attackDamage = 15;    // ile obrażeń zadaje ten przeciwnik
    AnimationState state = AnimationState::Idle;
    double hurtTimer = 0.0;
    double walkTimer = 0.0;
    bool mirrored = false;
};


