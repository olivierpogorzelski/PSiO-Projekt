#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"
#include "SkeletonSword.hpp"
#include "SkeletonBow.hpp"
#include "GoblinAxe.hpp"
#include "SmallDemon.hpp"
#include "BigDemon.hpp"
#include "SoundManager.hpp"
#include <random>
#include <ctime>

// ładowanie początkowego stanu mapy oraz wrogów
Map::Map() : worldMap{
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,9,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,9,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
} {
    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            if (worldMap[x][y] == 9) {
                bool isVert = false;
                if (x > 0 && x < mapWidth - 1) {
                    if (worldMap[x-1][y] >= 1 && worldMap[x-1][y] <= 8 && worldMap[x+1][y] >= 1 && worldMap[x+1][y] <= 8) {
                        isVert = true;
                    }
                }
                doors.push_back({x, y, 0, 0.0, isVert, 0.0});
            }
        }
    }

    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    std::uniform_real_distribution<double> distOffset(0.2, 0.8);
    std::uniform_int_distribution<int> distEnemyType(1, 5); 
    
    std::vector<std::vector<int>> rooms = {
        {8, 13, 18, 23},    // pokój 1
        {15, 22, 2, 7},     // pokój 2 (z kluczem)
        {25, 34, 2, 11},    // pokój 6
        {11, 16, 24, 29},   // pokój 3
        {8, 17, 31, 36},    // pokój 7
        {25, 30, 18, 25}    // pokój
    };
    std::uniform_int_distribution<int> distRoom(0, rooms.size() - 1);
    
    int spawnedEnemies = 0;
    while (spawnedEnemies < INITIAL_ENEMY_COUNT) {
        int r = distRoom(rng);
        std::uniform_int_distribution<int> distRx(rooms[r][0], rooms[r][1]);
        std::uniform_int_distribution<int> distRy(rooms[r][2], rooms[r][3]);
        
        int x = distRx(rng);
        int y = distRy(rng);
        
        if (worldMap[x][y] == 0) {
            double ex = x + distOffset(rng);
            double ey = y + distOffset(rng);
            int type = distEnemyType(rng);
            switch (type) {
                case 1: entities.push_back(std::make_unique<GoblinAxe>(ex, ey)); break;
                case 2: entities.push_back(std::make_unique<SmallDemon>(ex, ey)); break;
                case 3: entities.push_back(std::make_unique<Enemy>(ex, ey, 3)); break;
                case 4: entities.push_back(std::make_unique<SkeletonSword>(ex, ey)); break;
                case 5: entities.push_back(std::make_unique<SkeletonBow>(ex, ey)); break;
            }
            spawnedEnemies++;
        }
    }
    
    int spawnedPotions = 0;
    while (spawnedPotions < INITIAL_POTION_COUNT) {
        int r = distRoom(rng);
        std::uniform_int_distribution<int> distRx(rooms[r][0], rooms[r][1]);
        std::uniform_int_distribution<int> distRy(rooms[r][2], rooms[r][3]);
        int x = distRx(rng);
        int y = distRy(rng);
        if (worldMap[x][y] == 0) {
            entities.push_back(std::make_unique<Item>(x + 0.5, y + 0.5, 8)); 
            spawnedPotions++;
        }
    }
    
    // klucz w pokoju 2 - daleko od Bossa
    entities.push_back(std::make_unique<Item>(18.5, 4.5, 10)); 
    
    // boss w boss roomie (na wschodzie)
    entities.push_back(std::make_unique<BigDemon>(30.5, 35.5));
}

// pobiera wartość kafla ściany
int Map::getTile(int x, int y) const {
    if (x >= 0 && x < mapWidth && y >= 0 && y < mapHeight) {
        return worldMap[x][y];
    }
    return 1; // z definicji dajemy ścianę, by nie wyjść poza mapę
}

void Map::setTile(int x, int y, int value) {
    if (x >= 0 && x < mapWidth && y >= 0 && y < mapHeight) {
        worldMap[x][y] = value;
    }
}

bool Map::isWalkable(int x, int y) const {
    int tile = getTile(x, y);
    if (tile == 0) return true;
    if (tile == 9 || tile == 3) {
        const Door* door = getDoorAt(x, y);
        // jeśli drzwi są w pełni otwarte (offset bliski 1.0), można przejść
        if (door && door->offset >= 0.9) return true;
    }
    return false;
}

// logika drzwi
const std::vector<Door>& Map::getDoors() const {
    return doors;
}

Door* Map::getDoorAt(int x, int y) {
    for (auto& door : doors) {
        if (door.x == x && door.y == y) return &door;
    }
    return nullptr;
}

const Door* Map::getDoorAt(int x, int y) const {
    for (const auto& door : doors) {
        if (door.x == x && door.y == y) return &door;
    }
    return nullptr;
}

void Map::interactDoor(int x, int y) {
    Door* door = getDoorAt(x, y);
    if (door) {
        if (door->state == 0) {
            door->state = 1; // otwiera się
            SoundManager::getInstance().playSound("drzwi");
        } else if (door->state == 2) {
            door->state = 3; // zamyka
            SoundManager::getInstance().playSound("drzwi");
        } else if (door->state == 1) door->state = 3; // powraca
        else if (door->state == 3) door->state = 1; // powraca
    }
}

// zwraca szerokość mapy
int Map::getWidth() const {
    return mapWidth;
}

// zwraca wysokość mapy
int Map::getHeight() const {
    return mapHeight;
}

// pobiera cały wektor uniwersalny
std::vector<std::unique_ptr<GameObject>>& Map::getEntities() {
    return entities;
}

const std::vector<std::unique_ptr<GameObject>>& Map::getEntities() const {
    return entities;
}

// funkcje pomocnicze używajace DYNAMIC CAST

void Map::addProjectile(std::unique_ptr<Projectile> proj) {
    pendingEntities.push_back(std::move(proj));
}

// czyszczenie martwych wrogów i aktualizacja reszty
void Map::update(double frameTime, Player& player) { // przekazujemy całego gracza przez referencję
    // update drzwi
    for (auto& door : doors) {
        if (door.state == 1) { // otwieraja się
            door.offset += frameTime * 1.5;
            if (door.offset > 0.5) {
                if (worldMap[door.x][door.y] == 3 || worldMap[door.x][door.y] == 9) {
                    worldMap[door.x][door.y] = 0; // znikają w połowie otwarcia
                }
            }
            if (door.offset >= 1.0) {
                door.offset = 1.0;
                door.state = 2; // otwarte
                door.timer = 4.0; // 4 sekundy do zamknięcia
            }
        } else if (door.state == 2) { // otwarte
            door.timer -= frameTime;
            if (door.timer <= 0) {
                // sprawdź czy gracz lub wróg nie stoi w drzwiach
                bool entityInDoor = false;
                if ((int)player.getX() == door.x && (int)player.getY() == door.y) entityInDoor = true;
                for (const auto& obj : entities) {
                    if (auto enemy = dynamic_cast<Enemy*>(obj.get())) {
                        if ((int)enemy->getX() == door.x && (int)enemy->getY() == door.y && !enemy->isDead()) {
                            entityInDoor = true;
                            break;
                        }
                    }
                }
                
                if (!entityInDoor) {
                    door.state = 3; // zamykają się
                } else {
                    door.timer = 1.0; // poczekaj sekundę i spróbuj znów
                }
            }
        } else if (door.state == 3) { // zamykają się
            door.offset -= frameTime * 1.5;
            if (door.offset <= 0.5) {
                if (worldMap[door.x][door.y] == 0) {
                    worldMap[door.x][door.y] = 3; // pojawiają się znów jako zwykłe drzwi
                }
            }
            if (door.offset <= 0.0) {
                door.offset = 0.0;
                door.state = 0; // zamknięte
            }
        }
    }

    // update wszystkich entities używając polimorfizmu
    for (auto it = entities.begin(); it != entities.end(); ) {
        // jeśli obiekt zwróci true, zostaje usunięty (pocisk uderzył, item zebrany)
        if ((*it)->update(frameTime, player, *this)) {
            it = entities.erase(it);
        } else {
            ++it;
        }
    }
    
    // przeniesienie nowo utworzonych obiektów (np. pocisków) do głównej listy po aktualizacji
    for (auto& pending : pendingEntities) {
        entities.push_back(std::move(pending));
    }
    pendingEntities.clear();
}

// czysci wszystkie obiekty (do ładowania)
void Map::clearEntities() {
    entities.clear();
    for (auto& door : doors) {
        door.state = 0;
        door.offset = 0.0;
        door.timer = 0.0;
    }
}

// odtwarza wroga z zapisu
void Map::loadEnemy(double x, double y, int texture, int hp) {
    std::unique_ptr<Enemy> newEnemy;
    switch (texture) {
        case 0: newEnemy = std::make_unique<BigDemon>(x, y); break;
        case 1: newEnemy = std::make_unique<GoblinAxe>(x, y); break;
        case 2: newEnemy = std::make_unique<SmallDemon>(x, y); break;
        case 4: newEnemy = std::make_unique<SkeletonSword>(x, y); break;
        case 5: newEnemy = std::make_unique<SkeletonBow>(x, y); break;
        default: newEnemy = std::make_unique<Enemy>(x, y, texture); break; // w tym zjawa (3)
    }
    newEnemy->setHp(hp);
    newEnemy->setAttackTimer(2.0); // 2 sekundy ochrony po wczytaniu
    entities.push_back(std::move(newEnemy));
}

// odtwarza przedmiot z zapisu
void Map::loadItem(double x, double y, int texture, bool pickedUp) {
    if (!pickedUp) {
        auto item = std::make_unique<Item>(x, y, texture);
        entities.push_back(std::move(item));
    }
}
