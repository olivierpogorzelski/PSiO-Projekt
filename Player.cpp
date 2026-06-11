#include "Player.hpp"
#include "SoundManager.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <iostream>
#include <random>

// inicjalizacja położenia kamery i timera ataku
Player::Player(double startX, double startY, double startDirX, double startDirY, double startPlaneX, double startPlaneY)
    : posX(startX), posY(startY), dirX(startDirX), dirY(startDirY), planeX(startPlaneX), planeY(startPlaneY), attackTimer(0.0), hp(100)
{
    // domyślny ekwipunek startowy
    for(int i=0; i<5; i++) {
        inventoryWeapons[i] = 0;
        inventoryItems[i] = 0;
    }
    
    inventoryWeapons[0] = 1; // slot 1: miecz
    inventoryWeapons[1] = 2; // slot 2: kusza
    
    inventoryItems[0] = 1; // 1 domyślna potka na start
    inventoryItems[1] = 0;
    inventoryItems[2] = 0;
    
    activeWeapon = 0; // wybrany pierwszy slot (miecz)
}

// fizyka i wejście od gracza
void Player::update(double frameTime, Map& map) {
    // wykorzystanie prędkosci liniowej (w pikselach/sekunde) i rotacji w stopniach/sekunde
    double moveSpeedPxPerSec = 5.0;
    double rotationSpeedDegPerSec = 171.887; // ok. 3 rad/s
    
    double moveSpeed = frameTime * moveSpeedPxPerSec;
    double rotSpeed = frameTime * (rotationSpeedDegPerSec * M_PI / 180.0);

    // zmniejszanie licznika ataku
    if (attackTimer > 0.0) {
        attackTimer -= frameTime;
    }

    // chodzenie do przodu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        // automatyczne otwieranie zwykłych drzwi przed graczem
        int nextX = int(posX + dirX * moveSpeed * 3.0);
        int nextY = int(posY + dirY * moveSpeed * 3.0);
        if (map.getTile(nextX, nextY) == 3) {
            map.interactDoor(nextX, nextY);
        }
        
        if (map.isWalkable(int(posX + dirX * moveSpeed), int(posY))) posX += dirX * moveSpeed;
        if (map.isWalkable(int(posX), int(posY + dirY * moveSpeed))) posY += dirY * moveSpeed;
    }
    // chodzenie do tyłu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        if (map.isWalkable(int(posX - dirX * moveSpeed), int(posY))) posX -= dirX * moveSpeed;
        if (map.isWalkable(int(posX), int(posY - dirY * moveSpeed))) posY -= dirY * moveSpeed;
    }
    // obrót w prawo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(-rotSpeed) - dirY * std::sin(-rotSpeed);
        dirY = oldDirX * std::sin(-rotSpeed) + dirY * std::cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(-rotSpeed) - planeY * std::sin(-rotSpeed);
        planeY = oldPlaneX * std::sin(-rotSpeed) + planeY * std::cos(-rotSpeed);
    }
    // obrót w lewo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(rotSpeed) - dirY * std::sin(rotSpeed);
        dirY = oldDirX * std::sin(rotSpeed) + dirY * std::cos(rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(rotSpeed) - planeY * std::sin(rotSpeed);
        planeY = oldPlaneX * std::sin(rotSpeed) + planeY * std::cos(rotSpeed);
    }

    // interakcja przeniesiona na spacje w Player::attack
}

// próbuje uderzyć przeciwnika przed nami
void Player::attack(Map& map) {
    if (attackTimer > 0.0) return; // cooldown na atak
    
    int checkX = int(posX + dirX);
    int checkY = int(posY + dirY);
    
    // zawsze sprawdzaj czy przed nami są zwykłe drzwi (tile = 3)
    if (map.getTile(checkX, checkY) == 3) {
        map.interactDoor(checkX, checkY);
        attackTimer = 0.3;
        return;
    } else if (map.getTile(int(posX), int(posY)) == 3) {
        map.interactDoor(int(posX), int(posY));
        attackTimer = 0.3;
        return;
    }
    
    int wType = getActiveWeaponType();
    
    if (wType == 1) { // miecz
        attackTimer = 0.5; // czas animacji
        int r = rand() % 10 + 1;
        SoundManager::getInstance().playSound("miecz-cling" + std::to_string(r));
        
        auto& entities = map.getEntities();
        for (auto& entity : entities) {
            auto* enemy = dynamic_cast<Enemy*>(entity.get());
            if (!enemy) continue;
            
            double dx = enemy->getX() - posX;
            double dy = enemy->getY() - posY;
            double distance = std::sqrt(dx*dx + dy*dy);
            
            // uderzamy tylko jeśli jest wystarczajaco blisko i przed nami
            if (distance < 2.5 && !enemy->isDead()) {
                double dirToEnemyX = dx / distance;
                double dirToEnemyY = dy / distance;
                
                // sprawdzanie czy patrzymy w stronę wroga
                double dotProduct = dirToEnemyX * dirX + dirToEnemyY * dirY;
                if (dotProduct > 0.8) {
                    enemy->takeDamage(25); // zadajemy 25 pkt obrazen
                    if (enemy->isDead()) score += 10; // punkty za zabicie mieczem
                    break;
                }
            }
        }
    } else if (wType == 2) { // kusza
        attackTimer = 0.5; // czas przeładowania kuszy
        
        SoundManager::getInstance().playSound("strzala-wystrzal");
        // wystrzelenie lecącej strzały z pozycji gracza w kierunku, w którym patrzy
        map.addProjectile(std::make_unique<Projectile>(posX, posY, dirX, dirY, 9, 35, true));
    } else if (wType == 3) { // klucz
        attackTimer = 0.5;
        // sprawdź blok centralnie przed graczem
        if (map.getTile(checkX, checkY) == 9) {
            map.interactDoor(checkX, checkY); // otwieramy drzwi bossa
            map.setTile(checkX, checkY, 3); // zmieniamy je w zwykłe drzwi, by umożliwić przechodzenie (animacja zadziała)
        } else if (map.getTile(int(posX), int(posY)) == 9) {
            map.interactDoor(int(posX), int(posY));
            map.setTile(int(posX), int(posY), 3);
        }
    }
}

void Player::setActiveWeaponSlot(int slot) {
    if(slot >= 0 && slot < 5) {
        if(inventoryWeapons[slot] != 0) { // można zmienić tylko na zapełniony slot
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
            SoundManager::getInstance().playSound("gracz_potka");
            
            // "grawitacja" ekwipunku: przesuwamy wszystkie pozostałe potki slot wyżej
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
    if (hp <= 0) {
        hp = 0;
        SoundManager::getInstance().playSound("gracz_smierc");
    } else {
        SoundManager::getInstance().playSound("gracz_zraniony");
    }
}

void Player::addHp(int HP) {
    if (hp<100) {hp+=HP;}
    if(hp>100) {hp=100;}
}


