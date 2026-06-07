#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Map.hpp"
#include "Player.hpp"
#include "Constants.hpp"

// renderer odpowiedzialny za raycasting scian i sprite'ow
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
    
    struct SpriteFrame {
        std::vector<sf::Color> pixels;
        int width;
        int height;
        int bottomOffset;
        int xOffset;
        int blockH;
    };

    void loadTextures();
    void extractSpriteFrames(int textureIndex, const sf::Image& img);
    std::vector<sf::Color> scaleImage(const sf::Image& img, int targetW, int targetH);
    void setPixel(int x, int y, sf::Color color);
    void sortSprites(std::vector<int>& order, std::vector<double>& dist);
    void drawHud(sf::RenderWindow& window, const Player& player);
    void drawWeapon(sf::RenderWindow& window, const Player& player);

    std::vector<std::vector<sf::Color>> mapTextures;
    std::vector<std::vector<SpriteFrame>> spriteFrames; // [textureid][state]
    std::vector<sf::Uint8> screenPixels;
    sf::Texture screenTexture;
    sf::Sprite screenSprite;

    sf::Texture playerWeaponTexture;
    sf::Sprite playerWeaponSprite;
    std::vector<sf::IntRect> playerWeaponFrames;

    sf::Texture crossbowTexture;
    sf::Sprite crossbowSprite;
    std::vector<sf::IntRect> crossbowFrames;
    
    sf::Texture iconSwordTex;
    sf::Sprite iconSwordSprite;
    
    sf::Texture iconCrossbowTex;
    sf::Sprite iconCrossbowSprite;
    
    sf::Texture potkaTex;
    sf::Sprite potkaSprite;

    sf::Texture hudTexture;
    sf::Sprite hudSprite;
    sf::Font hudFont;
    
    // jednowymiarowy bufor glebokosci dla kazdej kolumny ekranu
    double zBuffer[bufferWidth];
};


