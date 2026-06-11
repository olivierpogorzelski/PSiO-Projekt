#include "BigDemon.hpp"

BigDemon::BigDemon(double startX, double startY) : Enemy(startX, startY, 0) // tekstura 0
{
    hp = 600;
    attackDamage = 40;
}


