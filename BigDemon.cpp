#include "BigDemon.hpp"

BigDemon::BigDemon(double startX, double startY) 
    : Enemy(startX, startY, 0) // texture 0
{
    hp = 150;
    attackDamage = 35;
}
