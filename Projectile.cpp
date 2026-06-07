#include "Projectile.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include <cmath>

Projectile::Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg, bool playerOwned)
    : x(startX), y(startY), dirX(directionX), dirY(directionY), texture(tex), damage(dmg), isPlayerOwned(playerOwned)
{
    speed = 3.0; // pociski leca szybciej niz gracz i wrogowie
    hitboxRadius = 0.3;
}

bool Projectile::update(double frameTime, Player& player, Map& map) {
    // ruch pocisku
    double nextX = x + dirX * speed * frameTime;
    double nextY = y + dirY * speed * frameTime;

    // sprawdzenie kolizji ze sciana
    if (map.getTile(int(nextX), int(nextY)) != 0) {
        return true; // zniszcz pocisk
    }

    x = nextX;
    y = nextY;

    if (isPlayerOwned) {
        // pocisk gracza: szuka kolizji z przeciwnikiem
        auto& enemies = map.getEnemies();
        for (auto& enemy : enemies) {
            if (enemy->isDead()) continue;
            double distToEnemyX = enemy->getX() - x;
            double distToEnemyY = enemy->getY() - y;
            double distanceToEnemy = std::sqrt(distToEnemyX * distToEnemyX + distToEnemyY * distToEnemyY);
            
            if (distanceToEnemy < hitboxRadius + 0.4) { // uderzenie w potwora
                enemy->takeDamage(damage);
                return true; // pocisk znika po uderzeniu
            }
        }
    } else {
        // pocisk wroga (magiczna kula): szuka kolizji z graczem
        double distToPlayerX = player.getX() - x;
        double distToPlayerY = player.getY() - y;
        double distanceToPlayer = std::sqrt(distToPlayerX * distToPlayerX + distToPlayerY * distToPlayerY);

        if (distanceToPlayer < hitboxRadius + 0.3) {
            player.takeDamage(damage);
            return true; // pocisk znika po uderzeniu w gracza
        }
    }

    return false; // kontynuuj lot
}


