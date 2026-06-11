#include "Projectile.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "SoundManager.hpp"
#include <cmath>

Projectile::Projectile(double startX, double startY, double directionX, double directionY, int tex, int dmg, bool playerOwned)
    : GameObject(startX, startY), dirX(directionX), dirY(directionY), texture(tex), damage(dmg), isPlayerOwned(playerOwned)
{
    moveSpeedPxPerSec = 3.0; // pociski lecą szybciej niż gracz i wrogowie
    hitboxRadius = 0.3;
}

bool Projectile::update(double frameTime, Player& player, Map& map) {
    // ruch pocisku
    double nextX = x + dirX * moveSpeedPxPerSec * frameTime;
    double nextY = y + dirY * moveSpeedPxPerSec * frameTime;

    // sprawdzenie kolizji ze ścianą
    if (map.getTile(int(nextX), int(nextY)) != 0) {
        if (isPlayerOwned) SoundManager::getInstance().playSound("strzala-trafienie");
        return true; // zniszcz pocisk
    }

    x = nextX;
    y = nextY;

    if (isPlayerOwned) {
        // pocisk gracza: szuka kolizji z przeciwnikiem
        auto& entities = map.getEntities();
        for (auto& entity : entities) {
            auto* enemy = dynamic_cast<Enemy*>(entity.get());
            if (!enemy) continue;

            if (!enemy->isDead()) {
                double dist = std::sqrt(std::pow(x - enemy->getX(), 2) + std::pow(y - enemy->getY(), 2));
                if (dist < hitboxRadius + 0.3) {
                    SoundManager::getInstance().playSound("strzala-trafienie");
                    enemy->takeDamage(damage);
                    if (enemy->isDead()) player.addScore(15); // punkty za zabicie kuszą
                    return true;
                }
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


