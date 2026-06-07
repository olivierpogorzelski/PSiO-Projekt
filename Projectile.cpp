#include "Projectile.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include <cmath>

Projectile::Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg)
    : x(startX), y(startY), dirX(directionX), dirY(directionY), texture(tex), damage(dmg)
{
    speed = 3.0; // pociski lecą szybciej niż gracz i wrogówie
    hitboxRadius = 0.3;
}

bool Projectile::update(double frameTime, Player& player, const Map& map) {
    // ruch pocisku
    double nextX = x + dirX * speed * frameTime;
    double nextY = y + dirY * speed * frameTime;

    // sprawdzenie kolizji ze ścianą
    if (map.getTile(int(nextX), int(nextY)) != 0) {
        return true; // zniszcz pocisk
    }

    x = nextX;
    y = nextY;

    // sprawdzenie kolizji z graczem
    double distToPlayerX = player.getX() - x;
    double distToPlayerY = player.getY() - y;
    double distanceToPlayer = std::sqrt(distToPlayerX * distToPlayerX + distToPlayerY * distToPlayerY);

    if (distanceToPlayer < hitboxRadius + 0.3) { // 0.3 to przybliżony promień gracza
        player.takeDamage(damage);
        return true; // zniszcz pocisk po trafieniu
    }

    return false; // kontynuuj lot
}
