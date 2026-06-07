#include "Enemy.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include <cmath>
// inicjalizacja wspolrzednych tekstury i zdrowia przeciwnika
Enemy::Enemy(double startX, double startY, int textureE)
    : x(startX), y(startY), texture(textureE), hp(100), attackTimer(0.0), attackDamage(15), state(AnimationState::Idle), hurtTimer(0.0) {}



void Enemy::update(double frameTime, Player& player, Map& map) {
    if (isDead()) {
        state = AnimationState::Dead;
        return;
    }

    if (hurtTimer > 0.0) {
        hurtTimer -= frameTime;
        state = AnimationState::Hurt;
    } else {
        state = AnimationState::Idle;
    }

    // zmniejszamy cooldown ataku w kazdej klatce
    if (attackTimer > 0.0) {
        attackTimer -= frameTime;
    }

    double dirX = player.getX() - x;
    double dirY = player.getY() - y;
    double distance = std::sqrt(dirX * dirX + dirY * dirY);

    bool isRanged = (texture == 3 || texture == 5);
    int projTex = (texture == 3) ? 6 : 7;
    double stopDistance = isRanged ? 5.0 : 0.8;
    double attackRange = isRanged ? 6.0 : 1.0;

    if (distance > 0) {
        dirX /= distance;
        dirY /= distance;
    }

    // 1. logika poruszania sie
    if (distance < 12.0 && distance > stopDistance) {
        if (hurtTimer <= 0.0) {
            state = AnimationState::Walk;
            walkTimer += frameTime;
            // zmieniamy odbicie lustrzane co 0.3 sekundy aby symulowac przebieranie nogami
            if (walkTimer > 0.3) {
                walkTimer = 0.0;
                mirrored = !mirrored;
            }
        }
        
        double moveSpeed = 0.8;
        double nextX = x + dirX * moveSpeed * frameTime;
        double nextY = y + dirY * moveSpeed * frameTime;

        double bufferX = (dirX > 0) ? 0.2 : -0.2;
        double bufferY = (dirY > 0) ? 0.2 : -0.2;

        if (map.getTile(int(nextX + bufferX), int(y)) == 0) {
            x = nextX;
        }
        if (map.getTile(int(x), int(nextY + bufferY)) == 0) {
            y = nextY;
        }
    }

    // 2. logika ataku
    if (distance <= attackRange) {
        if (hurtTimer <= 0.0) {
            if (attackTimer > 0.5) {
                state = AnimationState::Attack;
            } else {
                state = AnimationState::Idle;
            }
        }

        if (attackTimer <= 0.0) {
            if (isRanged) {
                // ranged: strzelamy pociskiem
                map.addProjectile(Projectile(x, y, dirX, dirY, projTex, attackDamage));
            } else {
                // melee: bezposrednie obrazenia
                if (distance <= 1.5) { // male zabezpieczenie, zeby melee nie bilo z 6 metrow w razie bledow
                    player.takeDamage(attackDamage);
                }
            }
            attackTimer = 1.0; // cooldown 1 s
            if (hurtTimer <= 0.0) state = AnimationState::Attack;
        }
    }
}
// zadawanie obrazen wrogowi
void Enemy::takeDamage(int amount) {
    hp -= amount;
    hurtTimer = 0.3; // pokazuj klatke otrzymania obrazen przez 0.3s
    
    if (hp <= 0) {
        hp = 0;
        state = AnimationState::Dead;
    }
}

// sprawdzanie czy zginal
bool Enemy::isDead() const {
    return hp <= 0;
}


