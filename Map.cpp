#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"
#include "SkeletonSword.hpp"
#include "SkeletonBow.hpp"
#include "GoblinAxe.hpp"
#include "SmallDemon.hpp"
#include "BigDemon.hpp"

// ladowanie poczatkowego stanu mapy oraz wrogow
/*Map::Map() : worldMap{
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
}*/
Map::Map() {
    clearEntities();
    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            worldMap[x][y] = 0;
        }
    }
}

// 2. Konstruktor jednoargumentowy, który po prostu wywołuje ładowanie
Map::Map(const std::string& filePath) {
    loadFromFile(filePath);
}

// 3. Główna funkcja ładująca / resetująca mapę
void Map::loadFromFile(const std::string& filePath) {
    // czyścimy listy przed każdym nowym ładowaniem (nowa gra lub load)
    clearEntities();
    enemies.clear();
    items.clear();

    // Czyszczenie tablicy kafelków na puste
    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            worldMap[x][y] = 0;
        }
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Blad wczytywania mapy: " << filePath << std::endl;
        return;
    }

    std::string header;
    while (file >> header) {
        if (header == "[MAP]"){
            int w = 0, h = 0;
            file >> w >> h; // wczytuje rozmiary, np. 24 24

            for (int y = 0; y < mapHeight; ++y) {
                for (int x = 0; x < mapWidth; ++x) {
                    int tileValue;
                    if (file >> tileValue) {
                        worldMap[y][x] = tileValue;
                    }
                }
            }
        }
        else if (header == "[ENEMIES]") {
            size_t count;
            if (file >> count) {
                for (size_t i = 0; i < count; ++i) {
                    double ex, ey;
                    int tex, hp;
                    if (file >> ex >> ey >> tex >> hp) {
                        if (tex == 1) {
                            enemies.push_back(std::make_unique<GoblinAxe>(ex, ey));
                        } else if (tex == 2) {
                            enemies.push_back(std::make_unique<SmallDemon>(ex, ey));
                        } else if (tex == 3) {
                            enemies.push_back(std::make_unique<Enemy>(ex, ey, 3));
                        } else if (tex == 4) {
                            enemies.push_back(std::make_unique<SkeletonSword>(ex, ey));
                        } else if (tex == 5) {
                            enemies.push_back(std::make_unique<SkeletonBow>(ex, ey));
                        } else if (tex == 0) {
                            enemies.push_back(std::make_unique<BigDemon>(ex, ey));
                        } else {
                            std::cerr << "Ostrzezenie: Nieznany typ przeciwnika (tex ID: " << tex << ")\n";
                        }

                        // Przypisanie wczytanego HP do nowo stworzonego potwora
                        if (!enemies.empty()) {
                            enemies.back()->setHp(hp);
                        }
                    }
                }
            }
        }
        else if (header == "[ITEMS]") {
            size_t count;
            if (file >> count) {
                for (size_t i = 0; i < count; ++i) {
                    float ix, iy;
                    int tex;
                    bool pickedUp;
                    if (file >> ix >> iy >> tex >> pickedUp) {
                        if (!pickedUp) {
                            items.push_back(Item(ix, iy, tex));
                        }
                    }
                }
            }
        }
    }

    file.close();
}

int Map::getTile(int x, int y) const {
    if (x >= 0 && x < mapWidth && y >= 0 && y < mapHeight) {
        return worldMap[x][y];
    }
    return 1;
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
    for (auto& enemy : enemies) {
        // Jeśli wróg nie żyje, ale JESZCZE nie wyrzucił przedmiotu
        if (enemy->isDead() && !enemy->hasSpawnedDrop()) {

            // Pobieramy przedmiot, który ma wypaść z tego konkretnego typu wroga
            int dropTex = enemy->getDropItemTexture();

            if (dropTex != 0) {
                // Tworzymy nowy przedmiot na ziemi w miejscu śmierci
                items.push_back(Item(enemy->getX(), enemy->getY(), dropTex));
            }

            // Oznaczamy wroga flagą true, aby w kolejnej klatce (update)
            // program nie stworzył kolejnej mikstury z tych samych zwłok
            enemy->setDropSpawned(true);
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
    default: newEnemy = std::make_unique<Enemy>(x, y, texture); break;
    }

    // 1. Nadpisujemy punkty życia z pliku zapisu
    newEnemy->setHp(hp);

    // 2. Jeśli wróg z zapisu jest martwy, konfigurujemy go jako zwłoki
    if (hp <= 0) {
        // Jeśli masz setter dla stanu:
        newEnemy->setState(AnimationState::Dead);
        // Jeśli pole state jest publiczne, to: newEnemy->state = AnimationState::Dead;

        // BLOKADA DRUGIEGO DROPU: informujemy grę, że ten wróg już wyrzucił potkę
        newEnemy->setDropSpawned(true);
    }

    enemies.push_back(std::move(newEnemy));
}

// odtwarza przedmiot z zapisu
void Map::loadItem(double x, double y, int texture, bool pickedUp) {
    // Jeśli przedmiot został już podniesiony w zapisanym stanie gry,
    // ignorujemy go i nie dodajemy z powrotem do świata.
    if (pickedUp) {
        return;
    }

    // Dodajemy tylko te przedmioty, które nadal leżą na ziemi
    Item item(x, y, texture);
    item.isPickedUp = false; // Skoro go dodajemy, to domyślnie leży na ziemi
    items.push_back(item);
}


