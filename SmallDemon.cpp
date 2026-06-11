#include "SmallDemon.hpp"

SmallDemon::SmallDemon(double startX, double startY) : Enemy(startX, startY, 2)
{
    hp = 45;
    attackDamage = 12;
}


