#include "Game.hpp"
#include "Constants.hpp"

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
    : window(sf::VideoMode(screenWidth, screenHeight), "Crimson Crypt", sf::Style::Fullscreen),
      player(22, 12, -1, 0, 0, 0.66), 
      state(GameState::menu),
      selectedMenuOption(0),
      currentDifficulty(Difficulty::Normal),
      selectedDifficultyOption(1)
{
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);
    initMenu();
}

void Game::initMenu() {
    if (!menuFont.loadFromFile(getAssetPath("textures/font.ttf"))) {
        // blad
    }
    if (!menuBgTexture.loadFromFile(getAssetPath("textures/menu_bg.png"))) {
        // blad
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
        // skaler zeby logo ladnie wypelnialo gorna czesc ekranu
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
    // resumetext pozycjonujemy dopiero w trybie pauzy

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

// glowna petla gry
void Game::run() {
    while (window.isOpen()) {
        processEvents();

        double frameTime = clock.restart().asSeconds();
        update(frameTime);

        render();
    }
}

// przetwarzanie wejscia uzytkownika i okna
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        // obsluga klawiszy wejsciowych (wcisniecie)
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
                // ekwipunek: uzycie potki
                else if (event.key.code == sf::Keyboard::Q) {
                    player.usePotion();
                }
            }
        }
        
        // obsluga klawiszy menu (puszczenie)
        if (event.type == sf::Event::KeyReleased) {
            // zabezpieczenie przed podwojnym kliknieciem (debounce) 
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
                    } else if (selectedMenuOption == 2) { // wyjdz
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
                    } else {
                        state = GameState::paused;
                    }
                }
            } else if (state == GameState::difficulty_selection) {
                if (event.key.code == sf::Keyboard::Up) {
                    selectedDifficultyOption--;
                    if (selectedDifficultyOption < 0) selectedDifficultyOption = 2;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    selectedDifficultyOption++;
                    if (selectedDifficultyOption > 2) selectedDifficultyOption = 0;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    if (selectedDifficultyOption == 0) currentDifficulty = Difficulty::Easy;
                    else if (selectedDifficultyOption == 1) currentDifficulty = Difficulty::Normal;
                    else if (selectedDifficultyOption == 2) currentDifficulty = Difficulty::Hard;
                    
                    saveGame(currentSlot); // natychmiastowy zapis by zajac slot
                    state = GameState::playing;
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
                    if (pausedMenuOption < 0) pausedMenuOption = 3;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    pausedMenuOption++;
                    if (pausedMenuOption > 3) pausedMenuOption = 0;
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
                    } else if (pausedMenuOption == 3) { // wyjdz do pulpitu
                        window.close();
                    }
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    state = GameState::playing; // powrot z pauzy tez klawiszem esc
                }
            }
        }
    }
}


// update logiki gry np fizyki ai zaleznie od stanu

void Game::update(double frameTime) {


    if (state == GameState::playing) {
        player.update(frameTime, map);
        map.update(frameTime, player); // przekazujemy obiekt gracza do mapy

        // opcjonalnie: jesli gracz zginie, zmien stan gry
        if (player.isDead()) {
            // state = gamestate::gameover; (gdy dorobisz taki stan)
            window.close(); // na razie po prostu zamykamy gre z braku ekranu smierci
        }
    }
}

// ============================================================================
// system zapisu i odczytu
// ============================================================================

std::string Game::getSaveInfo(int slot) {
    std::string filename = "save" + std::to_string(slot) + ".txt";
    struct stat result;
    if (stat(filename.c_str(), &result) == 0) {
        char buffer[100];
        // format: yyyy-mm-dd hh:mm
        strftime(buffer, sizeof(buffer), "(%Y-%m-%d %H:%M)", localtime(&result.st_mtime));
        
        // podglad pliku by wyciagnac poziom trudnosci
        std::string diffStr = "Normalny"; // domyslny fallback
        std::ifstream file(filename);
        if (file.is_open()) {
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
        }
        
        return "[" + diffStr + "] " + std::string(buffer);
    }
    return "[ Wolne ]";
}

void Game::updateSaveInfoCache() {
    for (int i = 1; i <= maxSlots; ++i) {
        cachedSaveInfo[i] = getSaveInfo(i);
    }
}

void Game::saveGame(int slot) {
    std::string filename = "save" + std::to_string(slot) + ".txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "Blad: Nie mozna utworzyc pliku " << filename << "!\n";
        return;
    }
    
    file << "[DIFFICULTY]\n";
    file << static_cast<int>(currentDifficulty) << "\n\n";
    
    file << "[PLAYER]\n";
    file << player.getX() << " " << player.getY() << " " 
         << player.getDirX() << " " << player.getDirY() << " "
         << player.getPlaneX() << " " << player.getPlaneY() << " "
         << player.getHp() << "\n\n";
         
    file << "[ENEMIES]\n";
    const auto& enemies = map.getEnemies();
    file << enemies.size() << "\n";
    for (auto& enemy : enemies) {
        file << enemy->getX() << " " << enemy->getY() << " " 
             << enemy->getTexture() << " " << enemy->getHp() << "\n";
    }
    file << "\n";
    
    file << "[ITEMS]\n";
    const auto& items = map.getItems();
    file << items.size() << "\n";
    for (const auto& item : items) {
        file << item.x << " " << item.y << " " 
             << item.texture << " " << item.isPickedUp << "\n";
    }
    file << "\n";
    
    file << "[INVENTORY]\n";
    file << player.activeWeapon << "\n";
    for(int i = 0; i < 5; i++) file << player.inventoryWeapons[i] << " ";
    file << "\n";
    for(int i = 0; i < 5; i++) file << player.inventoryItems[i] << " ";
    file << "\n";
    
    file.close();
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
            double px, py, dx, dy, plx, ply;
            int hp;
            file >> px >> py >> dx >> dy >> plx >> ply >> hp;
            player.setX(px);
            player.setY(py);
            player.setDirX(dx);
            player.setDirY(dy);
            player.setPlaneX(plx);
            player.setPlaneY(ply);
            player.setHp(hp);
        }
        else if (header == "[ENEMIES]") {
            size_t count;
            file >> count;
            map.clearEntities(); // zabezpieczenie usuwajace starych wrogow
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
            int aw;
            if (file >> aw) {
                player.activeWeapon = aw;
                for(int i=0; i<5; i++) {
                    if(!(file >> player.inventoryWeapons[i])) break;
                }
                for(int i=0; i<5; i++) {
                    if(!(file >> player.inventoryItems[i])) break;
                }
            }
        }
    }
    
    file.close();
    std::cout << "SUKCES: Gra wczytana poprawnie z pliku " << "save" + std::to_string(slot) + ".txt" << "!\n";
}
// rysowanie odpowiedniego ekranu
void Game::render() {
    if (state == GameState::menu) {
        window.clear();
        window.draw(menuBgSprite);
        
        window.draw(logoSprite);
        
        playText.setFillColor(selectedMenuOption == 0 ? sf::Color(255, 30, 30) : sf::Color(100, 100, 100));
        playText.setOutlineColor(sf::Color::Black);
        playText.setOutlineThickness(selectedMenuOption == 0 ? 3 : 0);
        
        loadText.setFillColor(selectedMenuOption == 1 ? sf::Color(255, 30, 30) : sf::Color(100, 100, 100));
        loadText.setOutlineColor(sf::Color::Black);
        loadText.setOutlineThickness(selectedMenuOption == 1 ? 3 : 0);
        
        exitText.setFillColor(selectedMenuOption == 2 ? sf::Color(255, 30, 30) : sf::Color(100, 100, 100));
        exitText.setOutlineColor(sf::Color::Black);
        exitText.setOutlineThickness(selectedMenuOption == 2 ? 3 : 0);
        
        // zabezpiecz pozycje dla glownego menu
        playText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 50);
        loadText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 120);
        exitText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 190);
        
        window.draw(playText);
        window.draw(loadText);
        window.draw(exitText);
        
        window.display();
    } else if (state == GameState::slot_selection) {
        // rysujemy tlo glownego menu lub gre z przyciemnieniem, w zaleznosci skad przyszlismy
        if (slotAction == SlotAction::NewGame || slotAction == SlotAction::LoadGame) {
            window.clear();
            window.draw(menuBgSprite);
            window.draw(logoSprite);
        } else {
            renderer.render(window, player, map);
            sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
            overlay.setFillColor(sf::Color(20, 0, 30, 200));
            window.draw(overlay);
        }
        
        // tytul ekranu
        sf::Text title;
        title.setFont(menuFont);
        title.setCharacterSize(50);
        if (slotAction == SlotAction::NewGame) title.setString("Wybierz Slot dla Nowej Gry");
        else if (slotAction == SlotAction::SaveGame) title.setString("Wybierz Slot do Zapisu");
        else title.setString("Wybierz Slot do Odczytu");
        
        sf::FloatRect tRect = title.getLocalBounds();
        title.setOrigin(tRect.left + tRect.width / 2.0f, tRect.top + tRect.height / 2.0f);
        title.setPosition(screenWidth / 2.0f, screenHeight / 2.0f - 260);
        title.setFillColor(sf::Color::White);
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(3);
        window.draw(title);
        
        // rysujemy 10 slotow w pionie
        for (int i = 1; i <= maxSlots; ++i) {
            sf::Text st;
            st.setFont(menuFont);
            st.setCharacterSize(40);
            
            std::string infoStr = cachedSaveInfo[i];
            
            if (i == selectedSlot) {
                st.setString("> Slot Zapisu " + std::to_string(i) + " " + infoStr + " <");
                st.setFillColor(sf::Color::Red);
            } else {
                st.setString("Slot Zapisu " + std::to_string(i) + " " + infoStr);
                st.setFillColor(sf::Color(150, 150, 150));
            }
            
            st.setOutlineColor(sf::Color::Black);
            st.setOutlineThickness(3);
            
            sf::FloatRect rect = st.getLocalBounds();
            st.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
            st.setPosition(screenWidth / 2.0f, screenHeight / 2.0f - 180 + (i - 1) * 45);
            
            window.draw(st);
        }
        
        window.display();
    } else if (state == GameState::difficulty_selection) {
        window.draw(menuBgSprite);
        window.draw(logoSprite);
        
        // ciemne tlo poprawiajace czytelnosc tak samo jak w slotach
        sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(80, 0, 0, 200));
        window.draw(overlay);
        
        sf::Text titleText;
        titleText.setFont(menuFont);
        titleText.setString("Wybierz Poziom Trudnosci");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color::White);
        sf::FloatRect trect = titleText.getLocalBounds();
        titleText.setOrigin(trect.left + trect.width / 2.0f, trect.top + trect.height / 2.0f);
        titleText.setPosition(screenWidth / 2.0f, screenHeight / 2.0f - 150);
        window.draw(titleText);
        
        std::vector<std::string> options = {"Latwy", "Normalny", "Trudny"};
        for (int i = 0; i < 3; ++i) {
            sf::Text opt;
            opt.setFont(menuFont);
            opt.setCharacterSize(50);
            
            if (i == selectedDifficultyOption) {
                opt.setString("> " + options[i] + " <");
                opt.setFillColor(sf::Color::Red);
            } else {
                opt.setString(options[i]);
                opt.setFillColor(sf::Color(150, 150, 150));
            }
            
            sf::FloatRect orect = opt.getLocalBounds();
            opt.setOrigin(orect.left + orect.width / 2.0f, orect.top + orect.height / 2.0f);
            opt.setPosition(screenWidth / 2.0f, screenHeight / 2.0f - 20 + i * 70);
            window.draw(opt);
        }
        
        window.display();
    } else if (state == GameState::playing) {
        renderer.render(window, player, map);
        window.display();
    } else if (state == GameState::paused) {
        // nakladamy ciemna warstwe na gre zeby zrobic wizualny efekt pauzy
        renderer.render(window, player, map);
        
        sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(80, 0, 0, 200)); // gleboki karmazynowy mrok pauzy
        window.draw(overlay);
        
        resumeText.setPosition(screenWidth/2.0f, screenHeight/2.0f - 100);
        saveText.setPosition(screenWidth/2.0f, screenHeight/2.0f - 30);
        loadText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 40);
        exitText.setPosition(screenWidth/2.0f, screenHeight/2.0f + 110);
        
        resumeText.setFillColor(pausedMenuOption == 0 ? sf::Color(255, 30, 30) : sf::Color(150, 150, 150));
        saveText.setFillColor(pausedMenuOption == 1 ? sf::Color(255, 30, 30) : sf::Color(150, 150, 150));
        loadText.setFillColor(pausedMenuOption == 2 ? sf::Color(255, 30, 30) : sf::Color(150, 150, 150));
        exitText.setFillColor(pausedMenuOption == 3 ? sf::Color(255, 30, 30) : sf::Color(150, 150, 150));
        
        resumeText.setOutlineColor(sf::Color::Black); resumeText.setOutlineThickness(pausedMenuOption == 0 ? 3 : 0);
        saveText.setOutlineColor(sf::Color::Black); saveText.setOutlineThickness(pausedMenuOption == 1 ? 3 : 0);
        loadText.setOutlineColor(sf::Color::Black); loadText.setOutlineThickness(pausedMenuOption == 2 ? 3 : 0);
        exitText.setOutlineColor(sf::Color::Black); exitText.setOutlineThickness(pausedMenuOption == 3 ? 3 : 0);
        
        window.draw(resumeText);
        window.draw(saveText);
        window.draw(loadText);
        window.draw(exitText);
        
        window.display();
    }
}


