#include "Enemy.hpp"
#include "Map.hpp"
#include "Player.hpp"
#include "SoundManager.hpp"
#include <cmath>
#include <random>

std::string Enemy::getSoundPrefix() const {
    switch(texture) {
        case 0: return "duzydemon_";
        case 1: return "goblin_";
        case 2: return "malydemon_";
        case 3: return "zjawa_";
        case 4: return "szkielet_";
        case 5: return "szkielet_";
        default: return "";
    }
}
// inicjalizacja współrzędnych tekstury i zdrowia przeciwnika
Enemy::Enemy(double startX, double startY, int textureE) : GameObject(startX, startY), texture(textureE), attackTimer(0.0), state(AnimationState::Idle), hurtTimer(0.0) {
    
    // Ustawienie statystyk zależnych od typu wroga (textureE)
    switch(textureE) {
        case 0: // Duży demon (Final Boss)
            hp = 600;
            attackDamage = 40;
            break;
        case 1: // Goblin
            hp = 100;
            attackDamage = 15;
            break;
        case 2: // Mały demon
            hp = 150;
            attackDamage = 20;
            break;
        case 3: // Zjawa
            hp = 120;
            attackDamage = 15;
            break;
        case 4: // Szkielet z łukiem
            hp = 80;
            attackDamage = 10;
            break;
        case 5: // Szkielet z mieczem
            hp = 100;
            attackDamage = 15;
            break;
        default:
            hp = 100;
            attackDamage = 15;
            break;
    }
}



bool Enemy::update(double frameTime, Player& player, Map& map) {
    if (isDead()) {
        state = AnimationState::Dead;
        return false;
    }

    if (hurtTimer > 0.0) {
        hurtTimer -= frameTime;
        state = AnimationState::Hurt;
    } else {
        state = AnimationState::Idle;
    }

    // zmniejszamy cooldown ataku w każdej klatce
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

    // 1. logika poruszania się
    if (distance < 12.0 && distance > stopDistance) {
        if (hurtTimer <= 0.0) {
            state = AnimationState::Walk;
            walkTimer += frameTime;
            // zmieniamy odbicie lustrzane co 0.3 sekundy aby symulować przebieranie nogami
            if (walkTimer > 0.3) {
                walkTimer = 0.0;
                mirrored = !mirrored;
                
                // Losowy dźwięk chodzenia / ambient "obok" z małą szansą
                if (rand() % 100 < 5) {
                    std::string pfx = getSoundPrefix();
                    if (!pfx.empty()) SoundManager::getInstance().playSound(pfx + "obok");
                }
            }
        }
        
        double moveSpeedPxPerSec = 0.8;
        double nextX = x + dirX * moveSpeedPxPerSec * frameTime;
        double nextY = y + dirY * moveSpeedPxPerSec * frameTime;

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
            std::string pfx = getSoundPrefix();
            if (isRanged) {
                // ranged: strzelamy pociskiem
                map.addProjectile(std::make_unique<Projectile>(x, y, dirX, dirY, projTex, attackDamage));
                
                if (texture == 3) {
                    SoundManager::getInstance().playSound(rand() % 2 == 0 ? "zjawa_atak-magia" : "zjawa_atak-magia2");
                } else if (texture == 5) {
                    SoundManager::getInstance().playSound("strzala-wystrzal");
                } else if (!pfx.empty()) {
                    SoundManager::getInstance().playSound(pfx + "atakdaleko");
                }
            } else {
                // melee: bezpośrednie obrażenia
                if (distance <= 1.5) { // małe zabezpieczenie, żeby melee nie biło z 6 metrów w razie błędów
                    player.takeDamage(attackDamage);
                    
                    if (texture == 4) {
                        SoundManager::getInstance().playSound("miecz-cling" + std::to_string(rand() % 10 + 1));
                    } else if (texture == 1) {
                        SoundManager::getInstance().playSound("goblin_topor" + std::to_string(rand() % 3 + 1));
                    } else if (!pfx.empty()) {
                        SoundManager::getInstance().playSound(pfx + "atakblisko");
                    }
                }
            }
            attackTimer = 1.0; // cooldown 1 s
            if (hurtTimer <= 0.0) state = AnimationState::Attack;
        }
    }
    
    return false;
}
// zadawanie obrażeń wrogowi
void Enemy::takeDamage(int amount) {
    hp -= amount;
    hurtTimer = 0.3; // pokazuj klatkę otrzymania obrażeń przez 0.3s
    
    std::string pfx = getSoundPrefix();
    
    if (hp <= 0) {
        hp = 0;
        state = AnimationState::Dead;
        if (!pfx.empty()) SoundManager::getInstance().playSound(pfx + "smierc");
    } else {
        if (!pfx.empty()) SoundManager::getInstance().playSound(pfx + "zraniony");
    }
}

// sprawdzanie czy zginął
bool Enemy::isDead() const {
    return hp <= 0;
}


