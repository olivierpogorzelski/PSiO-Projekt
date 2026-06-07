#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <iostream>

// inicjalizacja polozenia kamery i timera ataku
Player::Player(double startX, double startY, double startDirX, double startDirY, double startPlaneX, double startPlaneY)
    : posX(startX), posY(startY), dirX(startDirX), dirY(startDirY), planeX(startPlaneX), planeY(startPlaneY), attackTimer(0.0)
{
    // domyslny ekwipunek startowy
    for(int i=0; i<5; i++) {
        inventoryWeapons[i] = 0;
        inventoryItems[i] = 0;
    }
    
    inventoryWeapons[0] = 1; // slot 1: miecz
    inventoryWeapons[1] = 2; // slot 2: kusza
    
    inventoryItems[0] = 1; // 1 domyslna potka na start
    inventoryItems[1] = 0;
    inventoryItems[2] = 0;
    
    activeWeapon = 0; // wybrany pierwszy slot (miecz)
}

// fizyka i wejscie od gracza
void Player::update(double frameTime, const Map& map) {
    double moveSpeed = frameTime * 5.0;
    double rotSpeed = frameTime * 3.0;

    // zmniejszanie licznika ataku
    if (attackTimer > 0.0) {
        attackTimer -= frameTime;
    }

    // chodzenie do przodu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        if (map.getTile(int(posX + dirX * moveSpeed), int(posY)) == 0) posX += dirX * moveSpeed;
        if (map.getTile(int(posX), int(posY + dirY * moveSpeed)) == 0) posY += dirY * moveSpeed;
    }
    // chodzenie do tylu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        if (map.getTile(int(posX - dirX * moveSpeed), int(posY)) == 0) posX -= dirX * moveSpeed;
        if (map.getTile(int(posX), int(posY - dirY * moveSpeed)) == 0) posY -= dirY * moveSpeed;
    }
    // obrot w prawo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(-rotSpeed) - dirY * std::sin(-rotSpeed);
        dirY = oldDirX * std::sin(-rotSpeed) + dirY * std::cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(-rotSpeed) - planeY * std::sin(-rotSpeed);
        planeY = oldPlaneX * std::sin(-rotSpeed) + planeY * std::cos(-rotSpeed);
    }
    // obrot w lewo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(rotSpeed) - dirY * std::sin(rotSpeed);
        dirY = oldDirX * std::sin(rotSpeed) + dirY * std::cos(rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(rotSpeed) - planeY * std::sin(rotSpeed);
        planeY = oldPlaneX * std::sin(rotSpeed) + planeY * std::cos(rotSpeed);
    }
}

// probuje uderzyc przeciwnika przed nami
void Player::attack(Map& map) {
    if (attackTimer > 0.0) return; // cooldown na atak
    
    int wType = getActiveWeaponType();
    
    if (wType == 1) { // miecz
        attackTimer = 0.5; // czas animacji
        
        auto& enemies = map.getEnemies();
        for (auto& enemy : enemies) {
            double dx = enemy->getX() - posX;
            double dy = enemy->getY() - posY;
            double distance = std::sqrt(dx*dx + dy*dy);
            
            // uderzamy tylko jesli jest wystarczajaco blisko i przed nami
            if (distance < 2.5 && !enemy->isDead()) {
                double dirToEnemyX = dx / distance;
                double dirToEnemyY = dy / distance;
                
                // sprawdzanie czy patrzymy w strone wroga
                double dotProduct = dirToEnemyX * dirX + dirToEnemyY * dirY;
                if (dotProduct > 0.8) {
                    enemy->takeDamage(25); // zadajemy 25 pkt obrazen
                    break;
                }
            }
        }
    } else if (wType == 2) { // kusza
        attackTimer = 0.5; // czas przeladowania kuszy
        
        // wystrzelenie lecacej strzaly z pozycji gracza w kierunku, w ktorym patrzy
        map.addProjectile(Projectile(posX, posY, dirX, dirY, 7, 35, true));
    }
}

void Player::setActiveWeaponSlot(int slot) {
    if(slot >= 0 && slot < 5) {
        if(inventoryWeapons[slot] != 0) { // mozna zmienic tylko na zapelniony slot
            activeWeapon = slot;
            attackTimer = 0.0; // reset animacji po zmianie
        }
    }
}

int Player::getActiveWeaponType() const {
    return inventoryWeapons[activeWeapon];
}

void Player::usePotion() {
    for(int i = 0; i < 4; i++) {
        if(inventoryItems[i] == 1) {
            inventoryItems[i] = 0; // zuzywamy
            addHp(50);             // leczymy
            
            // "grawitacja" ekwipunku: przesuwamy wszystkie pozostale potki slot wyzej
            for(int j = i; j < 4; j++) {
                inventoryItems[j] = inventoryItems[j+1];
            }
            inventoryItems[4] = 0; // bezpieczne czyszczenie ostatniego indeksu tablicy
            
            std::cout << "Uzyto potki! HP = " << hp << "\n";
            return;
        }
    }
    std::cout << "Brak potek!\n";
}

void Player::takeDamage(int amount) {
    hp -= amount;
    if (hp < 0) hp = 0;
}

void Player::addHp(int HP) {
    if (hp<100) {hp+=HP;}
    if(hp>100) {hp=100;}
}


