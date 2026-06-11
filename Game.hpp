#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include "Map.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
#include "GameState.hpp"
#include "ScoreBoard.hpp"

// główna klasa silnika która zarządza grą


enum class SlotAction {
    NewGame,
    SaveGame,
    LoadGame
};

enum class Difficulty {
    Easy,
    Normal,
    Hard
};

/**
 * @class Game
 * @brief główna klasa zarządzająca całą logiką gry, oknem renderowania oraz stanowymi przejściami (menu, gra, game over).
 */
class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(double frameTime);
    void render();
    void initMenu();
    
    // metody do zapisu i odczytu gry
    void saveGame(int slot);
    void loadGame(int slot);
    std::string getSaveInfo(int slot);
    void updateSaveInfoCache();
    void updateSlotText();
    void updateSelectionText();

    sf::RenderWindow window;
    Map map;
    Player player;
    Renderer renderer;
    sf::Clock clock;
    sf::Clock inputClock;
    
    // menu
    sf::Font menuFont;
    sf::Texture menuBgTexture;
    sf::Sprite menuBgSprite;
    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    sf::Text playText;
    sf::Text exitText;
    sf::Text loadText;
    sf::Text saveText;
    sf::Text resumeText;
    sf::Text mainMenuText;
    sf::Text slotText;
    int selectedMenuOption;
    int pausedMenuOption;
    int gameOverMenuOption = 0;
    int gameWonMenuOption = 0;
    int currentSlot;
    
    // zmienne nowego ekranu wyboru slotów i trudności
    SlotAction slotAction;
    int selectedSlot;
    Difficulty currentDifficulty;
    int selectedDifficultyOption;
    std::string cachedSaveInfo[11];
    const int maxSlots = 10;
    
    ScoreBoard scoreBoard;
    bool showScoreBoard;
    
    // trzyma aktualny stan gry
    GameState state;
};


