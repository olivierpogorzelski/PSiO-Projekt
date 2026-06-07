#include "GoblinAxe.hpp"

GoblinAxe::GoblinAxe(double startX, double startY) 
    : Enemy(startX, startY, 1) // texture 1
{
    hp = 60;
    attackDamage = 20;
}
