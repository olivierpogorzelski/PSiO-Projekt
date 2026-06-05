#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"

// ładowanie początkowego stanu mapy oraz wrogów
Map::Map() : worldMap{
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,2,2,2,2,2,0,0,3,3,3,3,3,3,0,0,4,4,4,0,0,1},
        {1,0,2,0,0,0,0,2,0,0,3,0,0,0,0,3,0,0,4,0,4,0,0,1},
        {1,0,2,0,5,5,0,2,0,0,3,0,0,0,0,3,0,0,4,0,4,0,0,1},
        {1,0,2,0,5,5,0,2,0,0,3,3,0,3,3,3,0,0,4,4,4,0,0,1},
        {1,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,2,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,5,0,5,0,5,0,5,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,0,3,0,0,2,2,2,2,2,2,2,2,2,2,2,0,0,5,0,0,1},
        {1,0,3,0,3,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,3,0,3,0,0,2,0,0,4,0,0,0,0,4,0,2,0,0,5,0,0,1},
        {1,0,3,3,3,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,2,0,0,4,0,0,0,0,4,0,2,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,2,2,2,2,2,0,2,2,2,2,2,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
} {
    // inicjalizacja testowych wrogów
    enemies.push_back(Enemy(20.5, 11.5, 0)); // duzydemon
    enemies.push_back(Enemy(18.5, 4.5, 1));  // goblin
    enemies.push_back(Enemy(10.0, 4.5, 2));  // malydemon
    enemies.push_back(Enemy(10.0, 12.5, 3)); // zjawa
    enemies.push_back(Enemy(14.0, 10.5, 5)); // szkielet z lukiem
    items.push_back(Item(18.5,12.5,8));      // potka zdrowia ma indeks 8
}

// pobiera wartość kafla ściany
int Map::getTile(int x, int y) const {
    if (x >= 0 && x < mapWidth && y >= 0 && y < mapHeight) {
        return worldMap[x][y];
    }
    return 1; // z definicji dajemy ścianę by nie wyjść poza mapę
}

// zwraca szerokość mapy
int Map::getWidth() const {
    return mapWidth;
}

// zwraca wysokość mapy
int Map::getHeight() const {
    return mapHeight;
}

// zwraca wrogów do modyfikacji
std::vector<Enemy>& Map::getEnemies() {
    return enemies;
}

// zwraca wrogów do wyrenderowania
const std::vector<Enemy>& Map::getEnemies() const {
    return enemies;
}
std::vector<Item>& Map::getItems() {
    return items;
}
const std::vector<Item>& Map::getItems() const {
    return items;
}
std::vector<Projectile>& Map::getProjectiles() {
    return projectiles;
}

const std::vector<Projectile>& Map::getProjectiles() const {
    return projectiles;
}

void Map::addProjectile(const Projectile& proj) {
    projectiles.push_back(proj);
}

// czyszczenie martwych wrogów i aktualizacja reszty
void Map::update(double frameTime, Player& player) { // przekazujemy całego gracza przez referencję
    for (auto& enemy : enemies) {
        // martwi wrogowie nadal zajmują miejsce w pamięci, by móc ich wyrenderować jako zwłoki na podłodze.
        // nie wywołujemy dla nich update(), aby zoptymalizować działanie.
        if (!enemy.isDead()) {
            enemy.update(frameTime, player, *this);
        }
    }
    
    // aktualizacja pocisków
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        if (it->update(frameTime, player, *this)) {
            // zniszcz pocisk, jeśli uderzył w ścianę lub w gracza
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
    
    for (auto it = items.begin(); it != items.end(); ) {
        if (it->checkCollision(player.getX(), player.getY())) {
            player.addHp(20);
            it = items.erase(it); // usuwamy miksturę ze świata po zebraniu
        } else {
            ++it;
        }
    }
}
