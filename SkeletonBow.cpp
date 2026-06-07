#include "SkeletonBow.hpp"
#include "Player.hpp"
#include "Map.hpp"
#include <cmath>

SkeletonBow::SkeletonBow(double startX, double startY) 
    : Enemy(startX, startY, 5) // texture 5
{
    hp = 40;
    attackDamage = 10;
}

void SkeletonBow::update(double frameTime, Player& player, Map& map) {
    // na ten moment uzywamy bazowej aktualizacji, pozniej zaimplementujemy logike strzelania
    Enemy::update(frameTime, player, map);
}


