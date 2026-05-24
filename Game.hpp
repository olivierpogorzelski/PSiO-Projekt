#pragma once
#include <SFML/Graphics.hpp>
#include "Map.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
#include "GameState.hpp"

// główna klasa silnika która zarządza grą
class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(double frameTime);
    void render();

    sf::RenderWindow window;
    Map map;
    Player player;
    Renderer renderer;
    sf::Clock clock;
    
    // trzyma aktualny stan gry
    GameState state;
};
