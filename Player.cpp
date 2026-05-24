#include "Player.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

// inicjalizacja położenia kamery i timera ataku
Player::Player(double startX, double startY, double startDirX, double startDirY, double startPlaneX, double startPlaneY)
    : posX(startX), posY(startY), dirX(startDirX), dirY(startDirY), planeX(startPlaneX), planeY(startPlaneY), attackTimer(0.0) {}

// fizyka i wejście od gracza
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
    // chodzenie do tyłu
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        if (map.getTile(int(posX - dirX * moveSpeed), int(posY)) == 0) posX -= dirX * moveSpeed;
        if (map.getTile(int(posX), int(posY - dirY * moveSpeed)) == 0) posY -= dirY * moveSpeed;
    }
    // obrót w prawo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(-rotSpeed) - dirY * std::sin(-rotSpeed);
        dirY = oldDirX * std::sin(-rotSpeed) + dirY * std::cos(-rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(-rotSpeed) - planeY * std::sin(-rotSpeed);
        planeY = oldPlaneX * std::sin(-rotSpeed) + planeY * std::cos(-rotSpeed);
    }
    // obrót w lewo
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        double oldDirX = dirX;
        dirX = dirX * std::cos(rotSpeed) - dirY * std::sin(rotSpeed);
        dirY = oldDirX * std::sin(rotSpeed) + dirY * std::cos(rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * std::cos(rotSpeed) - planeY * std::sin(rotSpeed);
        planeY = oldPlaneX * std::sin(rotSpeed) + planeY * std::cos(rotSpeed);
    }
}

// próbuje uderzyć przeciwnika przed nami
void Player::attack(Map& map) {
    if (attackTimer > 0.0) return; // cooldown na atak
    
    attackTimer = 0.3; // czas animacji
    
    auto& enemies = map.getEnemies();
    for (auto& enemy : enemies) {
        double dx = enemy.getX() - posX;
        double dy = enemy.getY() - posY;
        double distance = std::sqrt(dx*dx + dy*dy);
        
        // sprawdzanie czy wróg jest blisko na dystans 1.5 kafla
        if (distance < 1.5) {
            // normalizacja wektora do wroga
            double nx = dx / distance;
            double ny = dy / distance;
            
            // sprawdzanie czy patrzymy w stronę wroga używając iloczynu skalarnego
            double dotProduct = nx * dirX + ny * dirY;
            if (dotProduct > 0.8) { // ok 36 stopni odchylenia
                enemy.takeDamage(50);
            }
        }
    }
}
