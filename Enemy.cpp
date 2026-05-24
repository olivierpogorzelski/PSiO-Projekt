#include "Enemy.hpp"

// inicjalizacja współrzędnych tekstury i zdrowia przeciwnika
Enemy::Enemy(double startX, double startY, int textureId)
    : x(startX), y(startY), texture(textureId), hp(100) {}

// w przyszłości znajdzie się tutaj jakieś ai
void Enemy::update(double frameTime) {
    // na razie nic tu nie ma
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
