#include "Map.hpp"
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
    enemies.push_back(Enemy(20.5, 11.5, 0));
    enemies.push_back(Enemy(18.5, 4.5, 1));
    enemies.push_back(Enemy(10.0, 4.5, 2));
    enemies.push_back(Enemy(10.0, 12.5, 3));
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

// czyszczenie martwych wrogów
/*void Map::update(double frameTime) {
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->isDead()) {
            it = enemies.erase(it);
        } else {
            it->update(frameTime);
            ++it;
        }
    }
}*/
void Map::update(double frameTime, Player& player) { // Przekazujemy całego gracza przez referencję
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->isDead()) {
            it = enemies.erase(it);
        } else {
            // Przekazujemy frameTime, gracza oraz tę mapę (*this)
            it->update(frameTime, player, *this);
            ++it;
        }
    }
}
