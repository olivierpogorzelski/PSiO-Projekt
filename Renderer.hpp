#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"

// renderer odpowiedzialny za raycasting ścian i sprite'ów
class Renderer {
public:
    Renderer();
    void render(sf::RenderWindow& window, const Player& player, const Map& map);


private:
    struct RenderableSprite {
        double x;
        double y;
        int textureIndex;
    };
    void generateTextures();
    void setPixel(int x, int y, sf::Color color);
    void sortSprites(std::vector<int>& order, std::vector<double>& dist);
    void drawHud(sf::RenderWindow& window, const Player& player);
    void drawWeapon(sf::RenderWindow& window, const Player& player);

    std::vector<std::vector<sf::Color>> textures;
    std::vector<sf::Uint8> screenPixels;
    sf::Texture screenTexture;
    sf::Sprite screenSprite;

    // jednowymiarowy bufor głębokości dla każdej kolumny ekranu
    double zBuffer[bufferWidth];
};
