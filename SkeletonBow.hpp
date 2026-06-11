#pragma once
#include "Enemy.hpp"

class SkeletonBow : public Enemy {
public:
    SkeletonBow(double startX, double startY);
    bool update(double frameTime, Player& player, Map& map) override;
};


