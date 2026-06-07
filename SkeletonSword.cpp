#include "SkeletonSword.hpp"

SkeletonSword::SkeletonSword(double startX, double startY) 
    : Enemy(startX, startY, 4) // texture 4
{
    hp = 50;
    attackDamage = 15;
}
