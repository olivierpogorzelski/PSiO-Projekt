#include "Enemy.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include <cmath>
// inicjalizacja współrzędnych tekstury i zdrowia przeciwnika
Enemy::Enemy(double startX, double startY, int textureE)
    : x(startX), y(startY), texture(textureE), hp(100), attackTimer(0.0), attackDamage(15) {}



void Enemy::update(double frameTime, Player& player, const Map& map) {
    if (isDead()) return;

    // Zmniejszamy cooldown ataku w każdej klatce
    if (attackTimer > 0.0) {
        attackTimer -= frameTime;
    }

    double dirX = player.getX() - x;
    double dirY = player.getY() - y;
    double distance = std::sqrt(dirX * dirX + dirY * dirY);

    // 1. Logika poruszania się (goni gracz, jeśli jest w zasięgu wzroku)
    if (distance < 12.0 && distance > 0.8) {
        dirX /= distance;
        dirY /= distance;

        double moveSpeed = 0.8;
        double nextX = x + dirX * moveSpeed * frameTime;
        double nextY = y + dirY * moveSpeed * frameTime;

        //if (map.getTile(int(nextX), int(y)) == 0) x = nextX;
        //if (map.getTile(int(x), int(nextY)) == 0) y = nextY;
        double bufferX = (dirX > 0) ? 0.2 : -0.2;
        double bufferY = (dirY > 0) ? 0.2 : -0.2;

        // Przy sprawdzaniu kafla mapy uwzględniamy wyliczony bufor
        if (map.getTile(int(nextX + bufferX), int(y)) == 0) {
            x = nextX;
        }
        if (map.getTile(int(x), int(nextY + bufferY)) == 0) {
            y = nextY;
        }
    }

    // 2. LOGIKA ATAKU: Jeśli wróg stoi tuż przy graczu i minął cooldown
    if (distance <= 1.0 && attackTimer <= 0.0) {
        player.takeDamage(attackDamage);
        attackTimer = 1.0; // Wróg może uderzyć ponownie dopiero za 1 sekundę
    }
}
// zadawanie obrażeń wrogowi
void Enemy::takeDamage(int amount) {
    hp -= amount;
    if (hp < 0) hp = 0;
}

// sprawdzanie czy zginął
bool Enemy::isDead() const {
    return hp <= 0;
}
