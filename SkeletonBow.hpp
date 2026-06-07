#pragma once
#include "Enemy.hpp"

class SkeletonBow : public Enemy {
public:
    SkeletonBow(double startX, double startY);
    void update(double frameTime, Player& player, Map& map) override;
};


