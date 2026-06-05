#include "Game.hpp"
#include "Constants.hpp"

// inicjalizacja okna gracza i stanu
Game::Game()
    : window(sf::VideoMode(screenWidth, screenHeight), "Test Raycastingu z Teksturami Naprawiony", sf::Style::Fullscreen),
      player(22, 12, -1, 0, 0, 0.66), 
      state(GameState::menu)
{
    window.setFramerateLimit(60);
}

// główna pętla gry
void Game::run() {
    while (window.isOpen()) {
        processEvents();

        double frameTime = clock.restart().asSeconds();
        update(frameTime);

        render();
    }
}

// przetwarzanie wejścia užytkownika i okna
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        
        // obsługa klawiszy wciśniętych raz
        if (event.type == sf::Event::KeyPressed) {
            if (state == GameState::menu) {
                if (event.key.code == sf::Keyboard::Enter) {
                    state = GameState::playing;
                }
            } else if (state == GameState::playing) {
                if (event.key.code == sf::Keyboard::Escape) {
                    state = GameState::paused;
                }
                // atak spacją
                if (event.key.code == sf::Keyboard::Space) {
                    player.attack(map);
                }
            } else if (state == GameState::paused) {
                    // jeśli gracz wciśnie enter wraca do gry
                    if (event.key.code == sf::Keyboard::Enter) {
                        state = GameState::playing;
                    }
                    // jeśli gracz wciśnie escape będąc w pauzie całkowicie zamykamy grę
                    else if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                }
            }
        }
    }


// update logiki gry np fizyki ai zaležnie od stanu

void Game::update(double frameTime) {
    if (state == GameState::playing) {
        player.update(frameTime, map);
        map.update(frameTime, player); // przekazujemy obiekt gracza do mapy

        // opcjonalnie: jeśli gracz zginie, zmień stan gry
        if (player.isDead()) {
            // state = gamestate::gameover; (gdy dorobisz taki stan)
            window.close(); // na razie po prostu zamykamy grę z braku ekranu śmierci
        }
    }
}
// rysowanie odpowiedniego ekranu
void Game::render() {
    if (state == GameState::menu) {
        // proste menu jako niebieskie tło dopóki nie mamy czcionki
        window.clear(sf::Color(0, 0, 100));
        window.display();
    } else if (state == GameState::playing) {
        renderer.render(window, player, map);
        window.display();
    } else if (state == GameState::paused) {
        // nakładamy ciemną warstwę na grę žeby zrobić wizualny efekt pauzy
        renderer.render(window, player, map);
        
        sf::RectangleShape overlay(sf::Vector2f(screenWidth, screenHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        
        window.display();
    }
}
