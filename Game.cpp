#include "Game.hpp"
#include "Constants.hpp"
#include "BigDemon.hpp"
#include "SoundManager.hpp"
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <ctime>

std::string getAssetPath(const std::string& filename) {
    std::vector<std::string> paths = {
        filename,
        "../" + filename,
        "../../" + filename,
        "../../../" + filename
    };
    for (const auto& p : paths) {
        std::ifstream f(p.c_str());
        if (f.good()) return p;
    }
    return filename;
}

// inicjalizacja okna gracza i stanu
Game::Game()
    : window(sf::VideoMode(screenWidth, screenHeight), "Crimson Crypt", sf::Style::Default),
      player(5.0, 5.0, 0.0, 1.0, 0.66, 0.0), 
      state(GameState::menu),
      selectedMenuOption(0),
      currentDifficulty(Difficulty::Normal),
      selectedDifficultyOption(1),
      showScoreBoard(false)
{
    SoundManager::getInstance().loadAllSounds();
    SoundManager::getInstance().playMusic("music/Title.ogg", false, 60.f);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    initMenu();
}

void Game::initMenu() {
    if (!menuFont.loadFromFile(getAssetPath("textures/font.ttf"))) {
        // błąd
    }
    if (!menuBgTexture.loadFromFile(getAssetPath("textures/menu_bg.png"))) {
        // błąd
    }
    menuBgSprite.setTexture(menuBgTexture);
    
    sf::Vector2u texSize = menuBgTexture.getSize();
    float scaleX = (float)screenWidth / texSize.x;
    float scaleY = (float)screenHeight / texSize.y;
    menuBgSprite.setScale(scaleX, scaleY);

    if (logoTexture.loadFromFile(getAssetPath("textures/crimsoncrypt.png"))) {
        logoSprite.setTexture(logoTexture);
        sf::FloatRect logoRect = logoSprite.getLocalBounds();
        logoSprite.setOrigin(logoRect.width / 2.0f, logoRect.height / 2.0f);
        // skalowanie żeby logo ładnie wypełniało górną część ekranu
        float maxLogoW = screenWidth * 0.8f;
        float maxLogoH = screenHeight * 0.4f;
        float logoScale = 1.0f;
        if (logoRect.width > maxLogoW || logoRect.height > maxLogoH) {
            float scaleW = maxLogoW / logoRect.width;
            float scaleH = maxLogoH / logoRect.height;
            logoScale = std::min(scaleW, scaleH);
        }
        logoSprite.setScale(logoScale, logoScale);
        logoSprite.setPosition(screenWidth / 2.0f, screenHeight / 4.0f);
    }

    playText.setFont(menuFont);
    playText.setString("Nowa Gra");
    playText.setCharacterSize(50);
    sf::FloatRect playRect = playText.getLocalBounds();
    playText.setOrigin(playRect.left + playRect.width/2.0f, playRect.top  + playRect.height/2.0f);
    playText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 50);

    loadText.setFont(menuFont);
    loadText.setString("Wczytaj Gre");
    loadText.setCharacterSize(50);
    sf::FloatRect loadRect = loadText.getLocalBounds();
    loadText.setOrigin(loadRect.left + loadRect.width/2.0f, loadRect.top  + loadRect.height/2.0f);
    loadText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 120);

    saveText.setFont(menuFont);
    saveText.setString("Zapisz Gre");
    saveText.setCharacterSize(50);
    sf::FloatRect saveRect = saveText.getLocalBounds();
    saveText.setOrigin(saveRect.left + saveRect.width/2.0f, saveRect.top  + saveRect.height/2.0f);
    // savetext pozycjonujemy dopiero w trybie pauzy

    resumeText.setFont(menuFont);
    resumeText.setString("Kontynuuj");
    resumeText.setCharacterSize(50);
    sf::FloatRect resRect = resumeText.getLocalBounds();
    resumeText.setOrigin(resRect.left + resRect.width/2.0f, resRect.top  + resRect.height/2.0f);
    // pozycjonujemy teksty dopiero w trybie pauzy

    mainMenuText.setFont(menuFont);
    mainMenuText.setString("Menu Glowne");
    mainMenuText.setCharacterSize(50);
    sf::FloatRect menuRect = mainMenuText.getLocalBounds();
    mainMenuText.setOrigin(menuRect.left + menuRect.width/2.0f, menuRect.top  + menuRect.height/2.0f);

    exitText.setFont(menuFont);
    exitText.setString("Wyjdz");
    exitText.setCharacterSize(50);
    sf::FloatRect exitRect = exitText.getLocalBounds();
    exitText.setOrigin(exitRect.left + exitRect.width/2.0f, exitRect.top  + exitRect.height/2.0f);
    exitText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 190);
    
    // inicjalizacja indeksu dla pauzy
    pausedMenuOption = 0;
    currentSlot = 1;
    selectedSlot = 1;
    
    slotText.setFont(menuFont);
    slotText.setCharacterSize(50);
}

// główna pętla gry
void Game::run() {
    while (window.isOpen()) {
        processEvents();

        double frameTime = clock.restart().asSeconds();
        if (frameTime > 0.1) frameTime = 0.1; // zapobieganie lag-spike'om i teleportacjom
        update(frameTime);

        render();
    }
}

// przetwarzanie wejścia użytkownika i okna
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        // obsługa klawiszy wejściowych (wciśnięcie)
        if (event.type == sf::Event::KeyPressed) {
            if (state == GameState::playing) {
                if (event.key.code == sf::Keyboard::Space) {
                    player.attack(map);
                }
                // ekwipunek: zmiana broni klawiszami 1-5
                else if (event.key.code == sf::Keyboard::Num1) player.setActiveWeaponSlot(0);
                else if (event.key.code == sf::Keyboard::Num2) player.setActiveWeaponSlot(1);
                else if (event.key.code == sf::Keyboard::Num3) player.setActiveWeaponSlot(2);
                else if (event.key.code == sf::Keyboard::Num4) player.setActiveWeaponSlot(3);
                else if (event.key.code == sf::Keyboard::Num5) player.setActiveWeaponSlot(4);
                // ekwipunek: użycie potki
                else if (event.key.code == sf::Keyboard::Q) {
                    player.usePotion();
                }
                else if (event.key.code == sf::Keyboard::Tab) {
                    showScoreBoard = true;
                }
                else if (event.key.code == sf::Keyboard::F5) {
                    saveGame(11); // slot 11 = quicksave
                }
                else if (event.key.code == sf::Keyboard::F9) {
                    loadGame(11); // slot 11 = quickload
                }
            }
        }
        
        if (event.type == sf::Event::KeyReleased) {
            if (state == GameState::playing && event.key.code == sf::Keyboard::Tab) {
                showScoreBoard = false;
            }
            // zabezpieczenie przed podwójnym kliknięciem (debounce)
            if (inputClock.getElapsedTime().asSeconds() < 0.25f) {
                continue; 
            }
            
            if (state == GameState::menu) {
                if (event.key.code == sf::Keyboard::Up) {
                    selectedMenuOption--;
                    if (selectedMenuOption < 0) selectedMenuOption = 2;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    selectedMenuOption++;
                    if (selectedMenuOption > 2) selectedMenuOption = 0;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    if (selectedMenuOption == 0) { // nowa gra
                        slotAction = SlotAction::NewGame;
                        updateSaveInfoCache();
                        state = GameState::slot_selection;
                    } else if (selectedMenuOption == 1) { // wczytaj
                        slotAction = SlotAction::LoadGame;
                        updateSaveInfoCache();
                        state = GameState::slot_selection;
                    } else if (selectedMenuOption == 2) { // wyjdź
                        window.close();
                    }
                }
            } else if (state == GameState::slot_selection) {
                if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Up) {
                    selectedSlot--;
                    if (selectedSlot < 1) selectedSlot = maxSlots;
                }
                else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Down) {
                    selectedSlot++;
                    if (selectedSlot > maxSlots) selectedSlot = 1;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    currentSlot = selectedSlot;
                    if (slotAction == SlotAction::NewGame) {
                        // opcjonalnie: reset mapy/gracza
                        state = GameState::difficulty_selection;
                        inputClock.restart();
                    } else if (slotAction == SlotAction::LoadGame) {
                        loadGame(currentSlot);
                        state = GameState::playing;
                        SoundManager::getInstance().playGameMusic();
                        inputClock.restart();
                    } else if (slotAction == SlotAction::SaveGame) {
                        saveGame(currentSlot);
                        state = GameState::paused;
                        inputClock.restart();
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    if (slotAction == SlotAction::NewGame || slotAction == SlotAction::LoadGame) {
                        state = GameState::menu;
                        SoundManager::getInstance().playMusic("music/Title.ogg", false, 60.f);
                    } else {
                        state = GameState::paused;
                    }
                }
            } else if (state == GameState::difficulty_selection) {
                // karty trudnosci sa poziomo - lewo/prawo, ale gora/dol tez dziala
                if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Up) {
                    selectedDifficultyOption--;
                    if (selectedDifficultyOption < 0) selectedDifficultyOption = 2;
                }
                else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Down) {
                    selectedDifficultyOption++;
                    if (selectedDifficultyOption > 2) selectedDifficultyOption = 0;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    if (selectedDifficultyOption == 0) currentDifficulty = Difficulty::Easy;
                    else if (selectedDifficultyOption == 1) currentDifficulty = Difficulty::Normal;
                    else if (selectedDifficultyOption == 2) currentDifficulty = Difficulty::Hard;
                    
                    saveGame(currentSlot); // natychmiastowy zapis by zająć slot
                    state = GameState::playing;
                    SoundManager::getInstance().playGameMusic();
                    inputClock.restart();
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    state = GameState::slot_selection;
                }
            } else if (state == GameState::playing) {
                if (event.key.code == sf::Keyboard::Escape) {
                    state = GameState::paused;
                }
            } else if (state == GameState::paused) {
                if (event.key.code == sf::Keyboard::Up) {
                    pausedMenuOption--;
                    if (pausedMenuOption < 0) pausedMenuOption = 4;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    pausedMenuOption++;
                    if (pausedMenuOption > 4) pausedMenuOption = 0;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    if (pausedMenuOption == 0) { // kontynuuj
                        state = GameState::playing;
                    } else if (pausedMenuOption == 1) { // zapisz
                        slotAction = SlotAction::SaveGame;
                        updateSaveInfoCache();
                        state = GameState::slot_selection;
                    } else if (pausedMenuOption == 2) { // wczytaj
                        slotAction = SlotAction::LoadGame;
                        updateSaveInfoCache();
                        state = GameState::slot_selection;
                    } else if (pausedMenuOption == 3) { // powrót do głównego menu
                        state = GameState::menu;
                    } else if (pausedMenuOption == 4) { // wyjdź do pulpitu
                        window.close();
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    state = GameState::playing; // powrót z pauzy też klawiszem esc
                }
            } else if (state == GameState::gameOver) {
                if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    gameOverMenuOption--;
                    if (gameOverMenuOption < 0) gameOverMenuOption = 2;
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    gameOverMenuOption++;
                    if (gameOverMenuOption > 2) gameOverMenuOption = 0;
                } else if (event.key.code == sf::Keyboard::Enter) {
                    if (gameOverMenuOption == 0) { // spróbuj ponownie
                        scoreBoard.addScore("Gracz", player.getScore());
                        // reset gry z nowym playerem w bezpiecznym miejscu
                        map = Map();
                        player = Player(5.0, 5.0, 0.0, 1.0, 0.66, 0.0);
                        state = GameState::playing;
                        SoundManager::getInstance().playGameMusic();
                    } else if (gameOverMenuOption == 1) { // wróć do menu
                        scoreBoard.addScore("Gracz", player.getScore());
                        map = Map();
                        player = Player(5.0, 5.0, 0.0, 1.0, 0.66, 0.0);
                        state = GameState::menu;
                        SoundManager::getInstance().playMusic("music/Title.ogg", false, 60.f);
                    } else if (gameOverMenuOption == 2) { // wyjdź z gry
                        scoreBoard.addScore("Gracz", player.getScore());
                        window.close();
                    }
                }
            } else if (state == GameState::gameWon) {
                if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    gameWonMenuOption--;
                    if (gameWonMenuOption < 0) gameWonMenuOption = 2;
                } else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    gameWonMenuOption++;
                    if (gameWonMenuOption > 2) gameWonMenuOption = 0;
                } else if (event.key.code == sf::Keyboard::Enter) {
                    if (gameWonMenuOption == 0) { // zagraj ponownie
                        scoreBoard.addScore("Gracz", player.getScore() + 1000); // bonus za wygraną
                        map = Map();
                        player = Player(5.0, 5.0, 0.0, 1.0, 0.66, 0.0);
                        state = GameState::playing;
                        SoundManager::getInstance().playGameMusic();
                    } else if (gameWonMenuOption == 1) { // wróć do menu
                        scoreBoard.addScore("Gracz", player.getScore() + 1000);
                        map = Map();
                        player = Player(5.0, 5.0, 0.0, 1.0, 0.66, 0.0);
                        state = GameState::menu;
                        SoundManager::getInstance().playMusic("music/Title.ogg", false, 60.f);
                    } else if (gameWonMenuOption == 2) { // wyjdź
                        scoreBoard.addScore("Gracz", player.getScore() + 1000);
                        window.close();
                    }
                }
            }
        }
    }
}


// update logiki gry np fizyki AI zależnie od stanu
void Game::update(double frameTime) {
    SoundManager::getInstance().update();
    if (state == GameState::playing) {
        if (player.isDead()) {
            state = GameState::gameOver;
        } else {
            bool bossDead = true;
            bool bossFound = false;
            auto& entities = map.getEntities();
            for (const auto& obj : entities) {
                if (auto bigDemon = dynamic_cast<BigDemon*>(obj.get())) {
                    bossFound = true;
                    if (!bigDemon->isDead()) {
                        bossDead = false;
                        break;
                    }
                }
            }
            if (bossFound && bossDead) {
                state = GameState::gameWon;
            } else {
                player.update(frameTime, map);
                map.update(frameTime, player);
            }
        }
    }
}


// system zapisu i odczytu
std::string Game::getSaveInfo(int slot) {
    std::string filename = "save" + std::to_string(slot) + ".txt";
    
    // podglad pliku by wyciagnac poziom trudnosci
    std::string diffStr = "Normalny"; // domyslny fallback
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "Pusty Slot";
    }
    
    std::string header;
    if (file >> header) {
        if (header == "[DIFFICULTY]") {
            int d;
            if (file >> d) {
                if (d == 0) diffStr = "Latwy";
                else if (d == 1) diffStr = "Normalny";
                else if (d == 2) diffStr = "Trudny";
            }
        }
    }
    
    file.close();
    return "[" + diffStr + "] Zapisano";
}

void Game::updateSaveInfoCache() {
    std::cout << "DEBUG: Wchodze do updateSaveInfoCache()\n"; std::cout.flush();
    for (int i = 1; i <= maxSlots; ++i) {
        std::cout << "DEBUG: Pobieram info dla slotu " << i << "\n"; std::cout.flush();
        try {
            cachedSaveInfo[i] = getSaveInfo(i);
        } catch(const std::exception& e) {
            std::cout << "WYJATEK: " << e.what() << "\n"; std::cout.flush();
        } catch(...) {
            std::cout << "WYJATEK NIEZNANY\n"; std::cout.flush();
        }
        std::cout << "DEBUG: Zapisano info dla slotu " << i << "\n"; std::cout.flush();
    }
    std::cout << "DEBUG: Wychodze z updateSaveInfoCache()\n"; std::cout.flush();
}

void Game::saveGame(int slot) {
    std::cout << "DEBUG: Zaczynam zapis do slotu " << slot << "\n"; std::cout.flush();
    std::string filename = "save" + std::to_string(slot) + ".txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Blad: Nie mozna utworzyc pliku " << filename << "!\n"; std::cout.flush();
        return;
    }
    
    std::cout << "DEBUG: Zapisuje trudnosc" << std::endl;
    file << "[DIFFICULTY]\n";
    file << static_cast<int>(currentDifficulty) << "\n\n";
    
    std::cout << "DEBUG: Zapisuje gracza" << std::endl;
    file << "[PLAYER]\n";
    file << player.getX() << " " << player.getY() << " " 
         << player.getDirX() << " " << player.getDirY() << " "
         << player.getPlaneX() << " " << player.getPlaneY() << " "
         << player.getHp() << " " << player.getScore() << "\n\n";
         
    std::cout << "DEBUG: Zapisuje wrogow" << std::endl;
    file << "[ENEMIES]\n";
    auto& entities = map.getEntities();
    
    // liczymy wrogów
    int enemyCount = 0;
    for (const auto& obj : entities) {
        if (dynamic_cast<Enemy*>(obj.get())) enemyCount++;
    }
    file << enemyCount << "\n";
    
    // zapisujemy wrogów
    for (const auto& obj : entities) {
        if (auto enemy = dynamic_cast<Enemy*>(obj.get())) {
            file << enemy->getX() << " " << enemy->getY() << " " 
                 << enemy->getTexture() << " " << enemy->getHp() << "\n";
        }
    }
    file << "\n";
    
    std::cout << "DEBUG: Zapisuje przedmioty" << std::endl;
    file << "[ITEMS]\n";
    // liczymy przedmioty
    int itemCount = 0;
    for (const auto& obj : entities) {
        if (dynamic_cast<Item*>(obj.get())) itemCount++;
    }
    file << itemCount << "\n";
    
    // zapisujemy przedmioty
    for (const auto& obj : entities) {
        if (auto item = dynamic_cast<Item*>(obj.get())) {
            // przedmiot zawsze ma pickedUp = false,
            // inaczej by go w ogóle nie było w kontenerze entities!
            file << item->getX() << " " << item->getY() << " " 
                 << item->getTexture() << " 0\n"; // 0 = false (nie podniesione)
        }
    }
    file << "\n";
    
    std::cout << "DEBUG: Zapisuje ekwipunek" << std::endl;
    file << "[INVENTORY]\n";
    file << player.activeWeapon << "\n";
    for(int i = 0; i < 5; i++) file << player.inventoryWeapons[i] << " ";
    file << "\n";
    for(int i = 0; i < 5; i++) file << player.inventoryItems[i] << " ";
    file << "\n";
    
    std::cout << "DEBUG: Zamykam plik" << std::endl;
    file.close();
    
    std::cout << "DEBUG: Aktualizuje cache" << std::endl;
    updateSaveInfoCache();
    std::cout << "SUKCES: Gra zostala zapisana do " << filename << "!\n";
}

void Game::loadGame(int slot) {
    std::string filename = "save" + std::to_string(slot) + ".txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Blad: Nie znaleziono pliku " << filename << " do wczytania!\n";
        return;
    }
    
    std::string header;
    while (file >> header) {
        if (header == "[DIFFICULTY]") {
            int d;
            file >> d;
            currentDifficulty = static_cast<Difficulty>(d);
        }
        else if (header == "[PLAYER]") {
            float px, py, dx, dy, plx, ply;
            int hp, score;
            file >> px >> py >> dx >> dy >> plx >> ply >> hp >> score;
            player.setX(px);
            player.setY(py);
            player.setDirX(dx);
            player.setDirY(dy);
            player.setPlaneX(plx);
            player.setPlaneY(ply);
            player.setHp(hp);
            player.setScore(score);
        }
        else if (header == "[ENEMIES]") {
            size_t count;
            file >> count;
            map.clearEntities(); // zabezpieczenie usuwające starych wrogów
            for (size_t i = 0; i < count; ++i) {
                double ex, ey;
                int tex, hp;
                file >> ex >> ey >> tex >> hp;
                map.loadEnemy(ex, ey, tex, hp);
            }
        }
        else if (header == "[ITEMS]") {
            size_t count;
            file >> count;
            for (size_t i = 0; i < count; ++i) {
                float ix, iy;
                int tex;
                bool pickedUp;
                file >> ix >> iy >> tex >> pickedUp;
                map.loadItem(ix, iy, tex, pickedUp);
            }
        }
        else if (header == "[INVENTORY]") {
            file >> player.activeWeapon;
            for (int i = 0; i < 5; ++i) {
                file >> player.inventoryWeapons[i];
            }
            for (int i = 0; i < 5; ++i) {
                file >> player.inventoryItems[i];
            }
        }
    }
    
    file.close();
    std::cout << "SUKCES: Gra wczytana poprawnie z pliku " << "save" + std::to_string(slot) + ".txt" << "!\n";
}
// rysowanie odpowiedniego ekranu
void Game::render() {
    if (state == GameState::playing) {
        renderer.render(window, player, map);
        
        if (showScoreBoard) {
            sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
            overlay.setFillColor(sf::Color(20, 0, 0, 220));
            window.draw(overlay);

            // górny pasek
            sf::RectangleShape topBar(sf::Vector2f(screenWidth * 0.7f, 4));
            topBar.setFillColor(sf::Color(180, 30, 0));
            topBar.setPosition(screenWidth * 0.15f, 85);
            window.draw(topBar);

            // tytuł
            sf::Text title;
            title.setFont(menuFont);
            title.setString("~  TABLICA WYNIKOW  ~");
            title.setCharacterSize(52);
            title.setFillColor(sf::Color(220, 170, 20));
            title.setOutlineColor(sf::Color::Black);
            title.setOutlineThickness(4);
            sf::FloatRect tRect = title.getLocalBounds();
            title.setOrigin(tRect.left + tRect.width / 2.0f, tRect.top + tRect.height / 2.0f);
            title.setPosition(screenWidth / 2.0f, 120);
            window.draw(title);

            // dolny pasek dekoracyjny pod tytułem
            sf::RectangleShape divider(sf::Vector2f(screenWidth * 0.7f, 4));
            divider.setFillColor(sf::Color(180, 30, 0));
            divider.setPosition(screenWidth * 0.15f, 155);
            window.draw(divider);

            const auto& scores = scoreBoard.getScores();
            if (scores.empty()) {
                sf::Text empty;
                empty.setFont(menuFont);
                empty.setString("Brak wynikow...");
                empty.setCharacterSize(36);
                empty.setFillColor(sf::Color(120, 80, 80));
                empty.setOutlineColor(sf::Color::Black);
                empty.setOutlineThickness(2);
                sf::FloatRect eRect = empty.getLocalBounds();
                empty.setOrigin(eRect.left + eRect.width / 2.0f, eRect.top + eRect.height / 2.0f);
                empty.setPosition(screenWidth / 2.0f, 250);
                window.draw(empty);
            }

            for (size_t i = 0; i < scores.size() && i < 10; ++i) {
                float yPos = 185 + i * 52;

                // podświetlenie co drugiego wiersza
                if (i % 2 == 0) {
                    sf::RectangleShape rowBg(sf::Vector2f(screenWidth * 0.6f, 46));
                    rowBg.setFillColor(sf::Color(60, 0, 0, 80));
                    rowBg.setPosition(screenWidth * 0.2f, yPos - 4);
                    window.draw(rowBg);
                }

                // kolor zależny od miejsca
                sf::Color entryColor;
                if (i == 0)      entryColor = sf::Color(255, 215, 0);   // zloty
                else if (i == 1) entryColor = sf::Color(220, 190, 140); // srebrny/bezowy
                else if (i == 2) entryColor = sf::Color(200, 120, 60);  // brazowy
                else             entryColor = sf::Color(200, 80, 80);   // ciemnoczerwony

                // numer miejsca
                sf::Text rank;
                rank.setFont(menuFont);
                rank.setString(std::to_string(i + 1) + ".");
                rank.setCharacterSize(38);
                rank.setFillColor(entryColor);
                rank.setOutlineColor(sf::Color::Black);
                rank.setOutlineThickness(2);
                rank.setPosition(screenWidth * 0.22f, yPos);
                window.draw(rank);

                // gracz
                sf::Text nameText;
                nameText.setFont(menuFont);
                nameText.setString(scores[i].name);
                nameText.setCharacterSize(38);
                nameText.setFillColor(entryColor);
                nameText.setOutlineColor(sf::Color::Black);
                nameText.setOutlineThickness(2);
                nameText.setPosition(screenWidth * 0.30f, yPos);
                window.draw(nameText);

                // wynik (wyrownany do prawej)
                sf::Text scoreText;
                scoreText.setFont(menuFont);
                scoreText.setString(std::to_string(scores[i].score));
                scoreText.setCharacterSize(38);
                scoreText.setFillColor(entryColor);
                scoreText.setOutlineColor(sf::Color::Black);
                scoreText.setOutlineThickness(2);
                sf::FloatRect sRect = scoreText.getLocalBounds();
                scoreText.setPosition(screenWidth * 0.78f - sRect.width, yPos);
                window.draw(scoreText);
            }

            // Dolny pasek dekoracyjny
            sf::RectangleShape bottomBar(sf::Vector2f(screenWidth * 0.7f, 4));
            bottomBar.setFillColor(sf::Color(180, 30, 0));
            bottomBar.setPosition(screenWidth * 0.15f, 185 + (int)std::min(scores.size(), (size_t)10) * 52 + 8);
            window.draw(bottomBar);

        }
        
        window.display();
    } else if (state == GameState::menu) {
        window.clear();
        window.draw(menuBgSprite);
        window.draw(logoSprite);

        // opcje menu blo
        auto drawMenuOpt = [&](sf::Text& txt, int idx, float y) {
            bool sel = (selectedMenuOption == idx);
            // wybrana opcja: złota i większa, reszta: przygaszona czerwień
            txt.setFillColor(sel ? sf::Color(220, 170, 20) : sf::Color(140, 70, 70));
            txt.setOutlineColor(sf::Color(0, 0, 0));
            txt.setOutlineThickness(sel ? 3 : 2);
            txt.setCharacterSize(sel ? 55 : 50);
            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
            txt.setPosition(screenWidth / 2.0f, y);
            window.draw(txt);
        };

        playText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 50);
        loadText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 120);
        exitText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 190);

        drawMenuOpt(playText, 0, screenHeight/2.0f + 50);
        drawMenuOpt(loadText, 1, screenHeight/2.0f + 120);
        drawMenuOpt(exitText, 2, screenHeight/2.0f + 190);

        window.display();
    } else if (state == GameState::gameWon) {
        // ekran wygranej - triumfalny, złoty. renderujemy grę w tle.
        window.clear();
        renderer.render(window, player, map);

        // ciepła złota poświata na cały ekran
        sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(40, 20, 0, 190));
        window.draw(overlay);

        // napis triumfu
        sf::Text wonText;
        wonText.setFont(menuFont);
        wonText.setString("ZWYCIESTWO");
        wonText.setCharacterSize(110);
        wonText.setFillColor(sf::Color(255, 210, 0));
        wonText.setOutlineColor(sf::Color(60, 30, 0));
        wonText.setOutlineThickness(7);
        sf::FloatRect wRect = wonText.getLocalBounds();
        wonText.setOrigin(wRect.left + wRect.width / 2.0f, wRect.top + wRect.height / 2.0f);
        wonText.setPosition(screenWidth / 2.0f, screenHeight * 0.28f);
        window.draw(wonText);

        // krótki opis fabularny
        sf::Text wonSub;
        wonSub.setFont(menuFont);
        wonSub.setString("Demon upadl. Krypta zostala oczyszczona.");
        wonSub.setCharacterSize(26);
        wonSub.setFillColor(sf::Color(180, 140, 60));
        wonSub.setOutlineColor(sf::Color::Black);
        wonSub.setOutlineThickness(2);
        sf::FloatRect wsRect = wonSub.getLocalBounds();
        wonSub.setOrigin(wsRect.left + wsRect.width / 2.0f, wsRect.top + wsRect.height / 2.0f);
        wonSub.setPosition(screenWidth / 2.0f, screenHeight * 0.42f);
        window.draw(wonSub);

        // ocje - duże, bez dekoracji, złoto/przygaszone
        std::vector<std::string> wonOpts = {"Zagraj Ponownie", "Wroc do menu", "Zakoncz gre"};
        for (int i = 0; i < 3; ++i) {
            sf::Text opt;
            opt.setFont(menuFont);
            bool sel = (gameWonMenuOption == i);
            opt.setCharacterSize(sel ? 52 : 44);
            opt.setString(wonOpts[i]);
            opt.setFillColor(sel ? sf::Color(255, 215, 0) : sf::Color(130, 100, 50));
            opt.setOutlineColor(sf::Color(0, 0, 0));
            opt.setOutlineThickness(sel ? 3 : 2);
            sf::FloatRect oRect = opt.getLocalBounds();
            opt.setOrigin(oRect.left + oRect.width / 2.0f, oRect.top + oRect.height / 2.0f);
            opt.setPosition(screenWidth / 2.0f, screenHeight * 0.58f + i * 68);
            window.draw(opt);
        }

        window.display();
    } else if (state == GameState::slot_selection) {
        // tło - menu bg lub gra w tle
        if (slotAction == SlotAction::NewGame || slotAction == SlotAction::LoadGame) {
            window.clear();
            window.draw(menuBgSprite);
            sf::RectangleShape bgDim(sf::Vector2f(screenWidth, screenHeight));
            bgDim.setFillColor(sf::Color(0, 0, 0, 140));
            window.draw(bgDim);
        } else {
            renderer.render(window, player, map);
            sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(overlay);
        }

        // panel centralny
        float panelW = screenWidth * 0.52f;
        float panelH = screenHeight * 0.78f;
        float panelX = (screenWidth - panelW) / 2.0f;
        float panelY = (screenHeight - panelH) / 2.0f;

        sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
        panel.setFillColor(sf::Color(12, 5, 5, 230));
        panel.setOutlineColor(sf::Color(120, 30, 10));
        panel.setOutlineThickness(2);
        panel.setPosition(panelX, panelY);
        window.draw(panel);

        // nagłówek panelu
        sf::Text title;
        title.setFont(menuFont);
        title.setCharacterSize(38);
        if (slotAction == SlotAction::NewGame) title.setString("Nowa Gra");
        else if (slotAction == SlotAction::SaveGame) title.setString("Zapisz Gre");
        else title.setString("Wczytaj Gre");
        title.setFillColor(sf::Color(200, 150, 20));
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(2);
        sf::FloatRect tRect = title.getLocalBounds();
        title.setOrigin(tRect.left + tRect.width / 2.0f, tRect.top + tRect.height / 2.0f);
        title.setPosition(screenWidth / 2.0f, panelY + 30);
        window.draw(title);

        // cieniutka linia pod nagłówkiem
        sf::RectangleShape hLine(sf::Vector2f(panelW - 40, 1));
        hLine.setFillColor(sf::Color(100, 40, 10, 180));
        hLine.setPosition(panelX + 20, panelY + 58);
        window.draw(hLine);

        // sloty, lista bez tabeli, czyste wpisy
        float slotStartY = panelY + 75;
        float slotStep = (panelH - 100) / (float)maxSlots;
        for (int i = 1; i <= maxSlots; ++i) {
            float ySlot = slotStartY + (i - 1) * slotStep;
            bool sel = (i == selectedSlot);

            // tylko wybrany slot ma tło
            if (sel) {
                sf::RectangleShape selBg(sf::Vector2f(panelW - 20, slotStep - 4));
                selBg.setFillColor(sf::Color(80, 20, 5, 130));
                selBg.setPosition(panelX + 10, ySlot - 2);
                window.draw(selBg);
            }

            sf::Text st;
            st.setFont(menuFont);
            st.setCharacterSize(sel ? 30 : 27);
            std::string infoStr = cachedSaveInfo[i];
            std::string slotLabel = (sel ? std::string("> ") : std::string("  ")) + "Slot " + std::to_string(i);
            st.setString(slotLabel);
            st.setFillColor(sel ? sf::Color(220, 170, 20) : sf::Color(130, 70, 60));
            st.setOutlineColor(sf::Color::Black);
            st.setOutlineThickness(sel ? 2 : 1);
            st.setPosition(panelX + 25, ySlot + (slotStep - st.getCharacterSize()) / 2.0f - 4);
            window.draw(st);

            // info o slocie wyrównane do prawej krawędzi panelu
            sf::Text info;
            info.setFont(menuFont);
            info.setCharacterSize(sel ? 27 : 24);
            info.setString(infoStr);
            info.setFillColor(sel ? sf::Color(180, 140, 60) : sf::Color(90, 55, 50));
            info.setOutlineColor(sf::Color::Black);
            info.setOutlineThickness(1);
            sf::FloatRect iRect = info.getLocalBounds();
            info.setPosition(panelX + panelW - iRect.width - 25, ySlot + (slotStep - info.getCharacterSize()) / 2.0f - 4);
            window.draw(info);
        }

        // podpowiedź pod panelem
        sf::Text slotHint;
        slotHint.setFont(menuFont);
        slotHint.setString("[GORA/DOL] Wybierz    [ENTER] Potwierdz    [ESC] Wstecz");
        slotHint.setCharacterSize(20);
        slotHint.setFillColor(sf::Color(90, 55, 55));
        slotHint.setOutlineColor(sf::Color::Black);
        slotHint.setOutlineThickness(1);
        sf::FloatRect shRect = slotHint.getLocalBounds();
        slotHint.setOrigin(shRect.left + shRect.width / 2.0f, shRect.top + shRect.height / 2.0f);
        slotHint.setPosition(screenWidth / 2.0f, panelY + panelH + 20);
        window.draw(slotHint);

        window.display();
    } else if (state == GameState::difficulty_selection) {
        // trudność - 3 wyraźnie oddzielne karty/bloki z wyborem
        window.clear();
        window.draw(menuBgSprite);

        sf::RectangleShape diffOverlay(sf::Vector2f(screenWidth, screenHeight));
        diffOverlay.setFillColor(sf::Color(0, 0, 0, 170));
        window.draw(diffOverlay);

        // tytuł - prosty, bez ozdobnikow
        sf::Text titleText;
        titleText.setFont(menuFont);
        titleText.setString("Wybierz poziom trudnosci");
        titleText.setCharacterSize(44);
        titleText.setFillColor(sf::Color(180, 140, 60));
        titleText.setOutlineColor(sf::Color::Black);
        titleText.setOutlineThickness(2);
        sf::FloatRect trect = titleText.getLocalBounds();
        titleText.setOrigin(trect.left + trect.width / 2.0f, trect.top + trect.height / 2.0f);
        titleText.setPosition(screenWidth / 2.0f, screenHeight * 0.18f);
        window.draw(titleText);

        // 3 odrębne karty trudności
        struct DiffCard { std::string name; std::string desc; sf::Color col; sf::Color bgCol; };
        std::vector<DiffCard> cards = {
            {"Latwy",   "Wrogowie slabsi,\nzdrowie sie regeneruje", sf::Color(80, 190, 80),  sf::Color(5, 30, 5)},
            {"Normalny","Standardowa\nrozgrywka",                  sf::Color(210, 160, 20), sf::Color(30, 20, 0)},
            {"Trudny",  "Maksymalne wyzwanie,\nbrak litosci",      sf::Color(210, 50, 50),  sf::Color(30, 0, 0)}
        };

        float cardW = screenWidth * 0.26f;
        float cardH = screenHeight * 0.38f;
        float spacing = screenWidth * 0.04f;
        float totalW = 3 * cardW + 2 * spacing;
        float startX = (screenWidth - totalW) / 2.0f;
        float cardY = screenHeight * 0.30f;

        for (int i = 0; i < 3; ++i) {
            bool sel = (i == selectedDifficultyOption);
            float cx = startX + i * (cardW + spacing);
            const auto& card = cards[i];

            // tło karty
            sf::RectangleShape cardBg(sf::Vector2f(cardW, cardH));
            cardBg.setFillColor(sel ? sf::Color(
                card.bgCol.r + 15, card.bgCol.g + 10, card.bgCol.b + 10, 240)
                : sf::Color(8, 3, 3, 200));
            cardBg.setOutlineColor(sel ? card.col : sf::Color(60, 30, 20));
            cardBg.setOutlineThickness(sel ? 3 : 1);
            cardBg.setPosition(cx, cardY);
            window.draw(cardBg);

            // nazwa trudności
            sf::Text nameT;
            nameT.setFont(menuFont);
            nameT.setCharacterSize(sel ? 46 : 40);
            nameT.setString(card.name);
            nameT.setFillColor(sel ? card.col : sf::Color(100, 70, 60));
            nameT.setOutlineColor(sf::Color::Black);
            nameT.setOutlineThickness(sel ? 3 : 2);
            sf::FloatRect nRect = nameT.getLocalBounds();
            nameT.setOrigin(nRect.left + nRect.width / 2.0f, nRect.top + nRect.height / 2.0f);
            nameT.setPosition(cx + cardW / 2.0f, cardY + cardH * 0.28f);
            window.draw(nameT);

            // opis
            if (sel) {
                sf::Text descT;
                descT.setFont(menuFont);
                descT.setCharacterSize(20);
                descT.setFillColor(sf::Color(160, 130, 80));
                descT.setOutlineColor(sf::Color::Black);
                descT.setOutlineThickness(1);

                std::string desc = card.desc;
                size_t nlPos = desc.find('\n');
                if (nlPos != std::string::npos) {
                    std::string line1 = desc.substr(0, nlPos);
                    std::string line2 = desc.substr(nlPos + 1);
                    
                    descT.setString(line1);
                    sf::FloatRect r1 = descT.getLocalBounds();
                    descT.setOrigin(r1.left + r1.width / 2.0f, r1.top + r1.height / 2.0f);
                    descT.setPosition(cx + cardW / 2.0f, cardY + cardH * 0.58f);
                    window.draw(descT);
                    
                    descT.setString(line2);
                    sf::FloatRect r2 = descT.getLocalBounds();
                    descT.setOrigin(r2.left + r2.width / 2.0f, r2.top + r2.height / 2.0f);
                    descT.setPosition(cx + cardW / 2.0f, cardY + cardH * 0.58f + 25.0f);
                    window.draw(descT);
                } else {
                    descT.setString(desc);
                    sf::FloatRect dRect = descT.getLocalBounds();
                    descT.setOrigin(dRect.left + dRect.width / 2.0f, dRect.top + dRect.height / 2.0f);
                    descT.setPosition(cx + cardW / 2.0f, cardY + cardH * 0.58f);
                    window.draw(descT);
                }
            }
        }

        // podpowiedzi
        sf::Text diffHint;
        diffHint.setFont(menuFont);
        diffHint.setString("[LEWO/PRAWO] Wybierz    [ENTER] Potwierdz");
        diffHint.setCharacterSize(20);
        diffHint.setFillColor(sf::Color(80, 55, 50));
        diffHint.setOutlineColor(sf::Color::Black);
        diffHint.setOutlineThickness(1);
        sf::FloatRect dhRect = diffHint.getLocalBounds();
        diffHint.setOrigin(dhRect.left + dhRect.width / 2.0f, dhRect.top + dhRect.height / 2.0f);
        diffHint.setPosition(screenWidth / 2.0f, screenHeight * 0.88f);
        window.draw(diffHint);

        window.display();
    } else if (state == GameState::paused) {
        // pauza - gra widoczna w tle, minimalny popup
        renderer.render(window, player, map);

        // lekkie przyciemnienie - gra wciąż widoczna
        sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 140));
        window.draw(overlay);

        // mały popup w centrum - nie zajmuje całego ekranu
        float popW = screenWidth * 0.32f;
        float popH = screenHeight * 0.46f;
        float popX = (screenWidth - popW) / 2.0f;
        float popY = (screenHeight - popH) / 2.0f;

        sf::RectangleShape popup(sf::Vector2f(popW, popH));
        popup.setFillColor(sf::Color(10, 3, 3, 240));
        popup.setOutlineColor(sf::Color(100, 30, 10));
        popup.setOutlineThickness(2);
        popup.setPosition(popX, popY);
        window.draw(popup);

        // Napis PAUZA na górze popupu
        sf::Text pauseTitle;
        pauseTitle.setFont(menuFont);
        pauseTitle.setString("PAUZA");
        pauseTitle.setCharacterSize(40);
        pauseTitle.setFillColor(sf::Color(180, 130, 20));
        pauseTitle.setOutlineColor(sf::Color::Black);
        pauseTitle.setOutlineThickness(2);
        sf::FloatRect ptRect = pauseTitle.getLocalBounds();
        pauseTitle.setOrigin(ptRect.left + ptRect.width / 2.0f, ptRect.top + ptRect.height / 2.0f);
        pauseTitle.setPosition(screenWidth / 2.0f, popY + 28);
        window.draw(pauseTitle);

        // opcje pauzy wewnątrz popupu
        auto drawPauseOpt = [&](sf::Text& txt, int idx, float y) {
            bool sel = (pausedMenuOption == idx);
            txt.setCharacterSize(sel ? 38 : 34);
            txt.setFillColor(sel ? sf::Color(220, 170, 20) : sf::Color(130, 70, 60));
            txt.setOutlineColor(sf::Color(0, 0, 0));
            txt.setOutlineThickness(sel ? 2 : 1);
            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
            txt.setPosition(screenWidth / 2.0f, y);
            window.draw(txt);
        };

        float optStart = popY + popH * 0.28f;
        float optStep  = popH * 0.14f;
        drawPauseOpt(resumeText,   0, optStart);
        drawPauseOpt(saveText,     1, optStart + optStep);
        drawPauseOpt(loadText,     2, optStart + optStep * 2);
        drawPauseOpt(mainMenuText, 3, optStart + optStep * 3);
        drawPauseOpt(exitText,     4, optStart + optStep * 4);

        window.display();
    } else if (state == GameState::gameOver) {
        // ekran śmierci
        window.clear();

        static sf::Texture goTex;
        static sf::Sprite goSprite;
        static bool goTexLoaded = false;
        if (!goTexLoaded) {
            if (goTex.loadFromFile(getAssetPath("textures/gameover_bg.png"))) {
                goSprite.setTexture(goTex);
                goSprite.setScale(
                    (float)screenWidth / goTex.getSize().x,
                    (float)screenHeight / goTex.getSize().y
                );
            }
            goTexLoaded = true;
        }
        window.draw(goSprite);

        // bardzo ciemna vignette
        sf::RectangleShape goOverlay(sf::Vector2f(screenWidth, screenHeight));
        goOverlay.setFillColor(sf::Color(5, 0, 0, 175));
        window.draw(goOverlay);

        // wielki napis
        sf::Text titleText;
        titleText.setFont(menuFont);
        titleText.setString("NIE ZYJESZ");
        titleText.setCharacterSize(120);
        titleText.setFillColor(sf::Color(190, 10, 10));
        titleText.setOutlineColor(sf::Color(0, 0, 0));
        titleText.setOutlineThickness(8);
        sf::FloatRect tBounds = titleText.getLocalBounds();
        titleText.setOrigin(tBounds.left + tBounds.width / 2.0f, tBounds.top + tBounds.height / 2.0f);
        titleText.setPosition(screenWidth / 2.0f, screenHeight * 0.30f);
        window.draw(titleText);

        // krótki flavour tekst
        sf::Text goSub;
        goSub.setFont(menuFont);
        goSub.setString("Twoja dusza przepadla w odmetach krypty...");
        goSub.setCharacterSize(24);
        goSub.setFillColor(sf::Color(110, 50, 50));
        goSub.setOutlineColor(sf::Color::Black);
        goSub.setOutlineThickness(1);
        sf::FloatRect gsRect = goSub.getLocalBounds();
        goSub.setOrigin(gsRect.left + gsRect.width / 2.0f, gsRect.top + gsRect.height / 2.0f);
        goSub.setPosition(screenWidth / 2.0f, screenHeight * 0.45f);
        window.draw(goSub);

        // opcje - proste, bez tła, sam tekst
        std::vector<std::string> opts = {"Sprobuj ponownie", "Wroc do menu", "Wyjdz"};
        for (int i = 0; i < 3; ++i) {
            bool sel = (i == gameOverMenuOption);
            sf::Text opt;
            opt.setFont(menuFont);
            opt.setCharacterSize(sel ? 50 : 42);
            opt.setString(opts[i]);
            opt.setFillColor(sel ? sf::Color(210, 40, 40) : sf::Color(110, 55, 55));
            opt.setOutlineColor(sf::Color(0, 0, 0));
            opt.setOutlineThickness(sel ? 3 : 2);
            sf::FloatRect oBounds = opt.getLocalBounds();
            opt.setOrigin(oBounds.left + oBounds.width / 2.0f, oBounds.top + oBounds.height / 2.0f);
            opt.setPosition(screenWidth / 2.0f, screenHeight * 0.58f + i * 66);
            window.draw(opt);
        }

        window.display();
    }
}


