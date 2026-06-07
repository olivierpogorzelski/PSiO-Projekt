#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"
#include "SkeletonSword.hpp"
#include "SkeletonBow.hpp"
#include "GoblinAxe.hpp"
#include "SmallDemon.hpp"
#include "BigDemon.hpp"

// ladowanie poczatkowego stanu mapy oraz wrogow
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
    // inicjalizacja testowych wrogow
    enemies.push_back(std::make_unique<Enemy>(20.5, 11.5, 3)); // zjawa zamiast duzego demona

    enemies.push_back(std::make_unique<GoblinAxe>(18.5, 4.5));  // goblin
    enemies.push_back(std::make_unique<SmallDemon>(10.0, 4.5));  // malydemon
    enemies.push_back(std::make_unique<Enemy>(10.0, 12.5, 3)); // zjawa
    enemies.push_back(std::make_unique<SkeletonBow>(14.0, 10.5)); // szkielet z lukiem
    
    // testowo mozna dodac tez innych jesli potrzeba
    // enemies.push_back(std::make_unique<bigdemon>(..., ...));
    // enemies.push_back(std::make_unique<skeletonsword>(..., ...));
    
    items.push_back(Item(18.5,12.5,8));      // potka zdrowia ma indeks 8
}

// pobiera wartosc kafla sciany
int Map::getTile(int x, int y) const {
    if (x >= 0 && x < mapWidth && y >= 0 && y < mapHeight) {
        return worldMap[x][y];
    }
    return 1; // z definicji dajemy sciane by nie wyjsc poza mape
}

// zwraca szerokosc mapy
int Map::getWidth() const {
    return mapWidth;
}

// zwraca wysokosc mapy
int Map::getHeight() const {
    return mapHeight;
}

// zwraca wrogow do modyfikacji
std::vector<std::unique_ptr<Enemy>>& Map::getEnemies() {
    return enemies;
}

// zwraca wrogow do wyrenderowania
const std::vector<std::unique_ptr<Enemy>>& Map::getEnemies() const {
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

// czyszczenie martwych wrogow i aktualizacja reszty
void Map::update(double frameTime, Player& player) { // przekazujemy calego gracza przez referencje
    // update wrogow
    for (auto& enemy : enemies) {
        // martwi wrogowie nadal zajmuja miejsce w pamieci, by moc ich wyrenderowac jako zwloki na podlodze.
        if (!enemy->isDead()) {
            enemy->update(frameTime, player, *this);
        }
    }
    
    // aktualizacja pociskow
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        if (it->update(frameTime, player, *this)) {
            // zniszcz pocisk, jesli uderzyl w sciane lub w gracza
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
    
    for (auto it = items.begin(); it != items.end(); ) {
        if (it->checkCollision(player.getX(), player.getY())) {
            bool pickedUp = false;
            for(int i = 0; i < 4; i++) {
                if(player.inventoryItems[i] == 0) { // szukamy pierwszego wolnego slota w plecaku
                    player.inventoryItems[i] = 1;
                    pickedUp = true;
                    break;
                }
            }
            
            if (pickedUp) {
                it = items.erase(it); // usuwamy miksture ze swiata, bo trafila do huda
            } else {
                ++it; // ekwipunek pelny, zostawiamy miksture na podlodze
            }
        } else {
            ++it;
        }
    }
}

// czysci wrogow, przedmioty i pociski podczas wczytywania gry
void Map::clearEntities() {
    enemies.clear();
    items.clear();
    projectiles.clear();
}

// odtwarza wroga z zapisu
void Map::loadEnemy(double x, double y, int texture, int hp) {
    std::unique_ptr<Enemy> newEnemy;
    
    switch (texture) {
        case 1: newEnemy = std::make_unique<GoblinAxe>(x, y); break;
        case 2: newEnemy = std::make_unique<SmallDemon>(x, y); break;
        case 4: newEnemy = std::make_unique<SkeletonSword>(x, y); break;
        case 5: newEnemy = std::make_unique<SkeletonBow>(x, y); break;
        default: newEnemy = std::make_unique<Enemy>(x, y, texture); break; // w tym zjawa (3)
    }
    
    // nadpisz domyslne hp punktami z save'a (wymaga sethp w enemy - zaraz dodamy)
    // aby to zadzialalo, musimy uzyc tymczasowego rzutowania lub po prostu przypisac to przez dedykowana metode
    newEnemy->setHp(hp);
    enemies.push_back(std::move(newEnemy));
}

// odtwarza przedmiot z zapisu
void Map::loadItem(double x, double y, int texture, bool pickedUp) {
    Item item(x, y, texture);
    item.isPickedUp = pickedUp;
    items.push_back(item);
}


