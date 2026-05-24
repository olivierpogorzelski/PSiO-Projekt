#include "Renderer.hpp"
#include <cmath>
#include <algorithm>

Renderer::Renderer()
    : textures(8, std::vector<sf::Color>(texWidth * texHeight)),
      screenPixels(bufferWidth * bufferHeight * 4, 0) 
{
    generateTextures();

    screenTexture.create(bufferWidth, bufferHeight);
    screenTexture.setSmooth(false);

    screenSprite.setTexture(screenTexture);
    screenSprite.setScale((float)screenWidth / bufferWidth, (float)screenHeight / bufferHeight);
}

// tymczasowe generowanie tekstur z braku własnych grafik
void Renderer::generateTextures() {
    for (int x = 0; x < texWidth; x++) {
        for (int y = 0; y < texHeight; y++) {
            int gridX = x / 8;
            int gridY = y / 8;
            bool isLight = (gridX + gridY) % 2 == 0;
            
            int color1 = isLight ? 200 : 100;
            int color2 = isLight ? 150 : 50;
            int color3 = isLight ? 255 : 128;
            
            textures[0][texWidth * y + x] = sf::Color(color3, color1, color1); // czerwona
            textures[1][texWidth * y + x] = sf::Color(color1, color3, color1); // zielona
            textures[2][texWidth * y + x] = sf::Color(color1, color1, color3); // niebieska
            textures[3][texWidth * y + x] = sf::Color(color2, color2, color2); // szara
            textures[4][texWidth * y + x] = sf::Color(color3, color3, color1); // żółta
            textures[5][texWidth * y + x] = sf::Color(isLight ? 90 : 60, isLight ? 90 : 60, isLight ? 90 : 60); // podłoga
            textures[6][texWidth * y + x] = sf::Color(isLight ? 50 : 30, isLight ? 50 : 30, isLight ? 50 : 30); // sufit
        }
    }
}

// ustawia pojedynczy piksel na buforze uwzględniając format rgba
void Renderer::setPixel(int x, int y, sf::Color color) {
    if (x < 0 || x >= bufferWidth || y < 0 || y >= bufferHeight) return;
    int index = (y * bufferWidth + x) * 4;
    screenPixels[index]     = color.r;
    screenPixels[index + 1] = color.g;
    screenPixels[index + 2] = color.b;
    screenPixels[index + 3] = 255;
}

// prosty algorytm sortujący wrogów od najdalszego do najbliższego
void Renderer::sortSprites(std::vector<int>& order, std::vector<double>& dist) {
    std::vector<std::pair<double, int>> sprites(order.size());
    for(size_t i = 0; i < order.size(); i++) {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }
    std::sort(sprites.begin(), sprites.end());
    // odwracamy żeby rysować najpierw to co z tyłu
    for(size_t i = 0; i < order.size(); i++) {
        dist[i] = sprites[order.size() - i - 1].first;
        order[i] = sprites[order.size() - i - 1].second;
    }
}

// główne renderowanie
void Renderer::render(sf::RenderWindow& window, const Player& player, const Map& map) {
    // czyszczenie bufora na czarno żeby uniknąć śmieci
    for(int i = 0; i < bufferWidth * bufferHeight * 4; i+=4) {
        screenPixels[i] = 0;
        screenPixels[i+1] = 0;
        screenPixels[i+2] = 0;
        screenPixels[i+3] = 255;
    }
    window.clear(sf::Color::Black);

    double posX = player.getX();
    double posY = player.getY();
    double dirX = player.getDirX();
    double dirY = player.getDirY();
    double planeX = player.getPlaneX();
    double planeY = player.getPlaneY();

    // pętla po każdej pionowej kolumnie ekranu
    for (int x = 0; x < bufferWidth; x++) {
        double cameraX = 2 * x / double(bufferWidth) - 1;
        double rayDirX = dirX + planeX * cameraX;
        double rayDirY = dirY + planeY * cameraX;

        int mapX = int(posX);
        int mapY = int(posY);

        double sideDistX;
        double sideDistY;
        double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1/rayDirX);
        double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1/rayDirY);
        double perpWallDist;

        int stepX;
        int stepY;
        int hit = 0;
        int side = 0;

        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        // algorytm dda szukający zderzenia promienia ze ścianą
        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (map.getTile(mapX, mapY) > 0) hit = 1;
        }

        if(side == 0) perpWallDist = (sideDistX - deltaDistX);
        else          perpWallDist = (sideDistY - deltaDistY);

        int lineHeight = (int)(bufferHeight / perpWallDist);

        int drawStart = -lineHeight / 2 + bufferHeight / 2;
        if(drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + bufferHeight / 2;
        if(drawEnd >= bufferHeight) drawEnd = bufferHeight - 1;

        int texNum = map.getTile(mapX, mapY) - 1;

        double wallX;
        if (side == 0) wallX = posY + perpWallDist * rayDirY;
        else           wallX = posX + perpWallDist * rayDirX;
        wallX -= std::floor((wallX));

        int texX = int(wallX * double(texWidth));
        if(side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
        if(side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

        double step = 1.0 * texHeight / lineHeight;
        double texPos = (drawStart - bufferHeight / 2 + lineHeight / 2) * step;

        double floorXWall, floorYWall;
        if(side == 0 && rayDirX > 0) { floorXWall = mapX; floorYWall = mapY + wallX; }
        else if(side == 0 && rayDirX < 0) { floorXWall = mapX + 1.0; floorYWall = mapY + wallX; }
        else if(side == 1 && rayDirY > 0) { floorXWall = mapX + wallX; floorYWall = mapY; }
        else { floorXWall = mapX + wallX; floorYWall = mapY + 1.0; }

        // rysowanie sufitu
        for (int y = 0; y < drawStart; y++) {
            int floorY = bufferHeight - y - 1;
            if (2 * floorY - bufferHeight == 0) continue;

            double currentDist = bufferHeight / (2.0 * floorY - bufferHeight);
            double weight = currentDist / perpWallDist;

            double currentCeilX = weight * floorXWall + (1.0 - weight) * posX;
            double currentCeilY = weight * floorYWall + (1.0 - weight) * posY;

            int ceilTexX = int(currentCeilX * texWidth) % texWidth;
            int ceilTexY = int(currentCeilY * texHeight) % texHeight;
            if(ceilTexX < 0) ceilTexX += texWidth;
            if(ceilTexY < 0) ceilTexY += texHeight;

            setPixel(x, y, textures[6][texWidth * ceilTexY + ceilTexX]);
        }

        // rysowanie ściany
        for (int y = drawStart; y < drawEnd; y++) {
            int texY = (int)texPos & (texHeight - 1);
            texPos += step;
            sf::Color color = textures[texNum][texWidth * texY + texX];

            // przyciemnienie ścian żeby dać iluzję cienia
            if (side == 1) {
                color.r /= 2; color.g /= 2; color.b /= 2;
            }

            setPixel(x, y, color);
        }

        // rysowanie podłogi
        for(int y = drawEnd; y < bufferHeight; y++) {
            if (2 * y - bufferHeight == 0) continue;

            double currentDist = bufferHeight / (2.0 * y - bufferHeight);
            double weight = currentDist / perpWallDist;

            double currentFloorX = weight * floorXWall + (1.0 - weight) * posX;
            double currentFloorY = weight * floorYWall + (1.0 - weight) * posY;

            int floorTexX = int(currentFloorX * texWidth) % texWidth;
            int floorTexY = int(currentFloorY * texHeight) % texHeight;
            if(floorTexX < 0) floorTexX += texWidth;
            if(floorTexY < 0) floorTexY += texHeight;

            setPixel(x, y, textures[5][texWidth * floorTexY + floorTexX]);
        }
        
        // zapisanie dystansu do ściany dla danego x ekranu na potrzeby sortowania dogłębnego sprite'ów
        zBuffer[x] = perpWallDist;
    }

    // rysowanie wrogów metodą lodeva
    const auto& enemies = map.getEnemies();
    size_t numEnemies = enemies.size();
    
    std::vector<int> enemyOrder(numEnemies);
    std::vector<double> enemyDistance(numEnemies);
    
    for(size_t i = 0; i < numEnemies; i++) {
        enemyOrder[i] = i;
        enemyDistance[i] = ((posX - enemies[i].getX()) * (posX - enemies[i].getX()) + (posY - enemies[i].getY()) * (posY - enemies[i].getY()));
    }
    
    sortSprites(enemyOrder, enemyDistance);
    
    for(size_t i = 0; i < numEnemies; i++) {
        double spriteX = enemies[enemyOrder[i]].getX() - posX;
        double spriteY = enemies[enemyOrder[i]].getY() - posY;
        
        // macierz odwrotnej kamery żeby przetłumaczyć to na piksele ekranowe
        double invDet = 1.0 / (planeX * dirY - dirX * planeY);
        double transformX = invDet * (dirY * spriteX - dirX * spriteY);
        double transformY = invDet * (-planeY * spriteX + planeX * spriteY);
        
        int spriteScreenX = int((bufferWidth / 2) * (1 + transformX / transformY));
        
        // wymiary spritea opierają się o transformy żeby nie było efektu rybiego oka
        int spriteHeight = std::abs(int(bufferHeight / transformY));
        int drawStartY = -spriteHeight / 2 + bufferHeight / 2;
        if(drawStartY < 0) drawStartY = 0;
        int drawEndY = spriteHeight / 2 + bufferHeight / 2;
        if(drawEndY >= bufferHeight) drawEndY = bufferHeight - 1;
        
        int spriteWidth = std::abs(int(bufferHeight / transformY));
        int drawStartX = -spriteWidth / 2 + spriteScreenX;
        if(drawStartX < 0) drawStartX = 0;
        int drawEndX = spriteWidth / 2 + spriteScreenX;
        if(drawEndX >= bufferWidth) drawEndX = bufferWidth - 1;
        
        for(int stripe = drawStartX; stripe < drawEndX; stripe++) {
            int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth) / 256;
            
            // czy mieści się przed kamerą na ekranie oraz czy nie chowa się za ścianą
            if(transformY > 0 && stripe > 0 && stripe < bufferWidth && transformY < zBuffer[stripe]) {
                for(int y = drawStartY; y < drawEndY; y++) {
                    int d = (y) * 256 - bufferHeight * 128 + spriteHeight * 128;
                    int texY = ((d * texHeight) / spriteHeight) / 256;
                    
                    sf::Color color = textures[enemies[enemyOrder[i]].getTexture()][texWidth * texY + texX];
                    
                    // pomijamy kolor jeśli jego alfa jest równa zero albo używamy koloru czarnego jako tło zakładamy że czysty czarny to brak piksela
                    if(color.r != 0 || color.g != 0 || color.b != 0) {
                        setPixel(stripe, y, color);
                    }
                }
            }
        }
    }

    screenTexture.update(screenPixels.data());
    window.draw(screenSprite);
    
    drawWeapon(window, player);
    drawHud(window);
}

// rysowanie bazowego interfejsu
void Renderer::drawHud(sf::RenderWindow& window) {
    sf::RectangleShape hudBar(sf::Vector2f(1920.0f, 80.0f));
    hudBar.setPosition(0.0f, 1000.0f);
    hudBar.setFillColor(sf::Color(30, 30, 30));
    
    // prosty pasek zdrowia tylko wizualny na razie
    sf::RectangleShape healthBar(sf::Vector2f(300.0f, 40.0f));
    healthBar.setPosition(50.0f, 1020.0f);
    healthBar.setFillColor(sf::Color(200, 0, 0));
    
    window.draw(hudBar);
    window.draw(healthBar);
}

// rysowanie aktywnej broni zależnie od stanu gracza
void Renderer::drawWeapon(sf::RenderWindow& window, const Player& player) {
    // jeden prosty miecz po prawej stronie ekranu zrobiony z wielokąta
    sf::ConvexShape sword;
    sword.setPointCount(4);
    
    // efekt machnięcia bazujący na timerze z gracza
    double animOffset = 0.0;
    if (player.getAttackTimer() > 0.0) {
        animOffset = std::sin(player.getAttackTimer() / 0.3 * 3.14159) * 150.0;
    }
    
    sword.setPoint(0, sf::Vector2f(1400.0f - animOffset, 1000.0f));
    sword.setPoint(1, sf::Vector2f(1600.0f - animOffset, 600.0f + animOffset));
    sword.setPoint(2, sf::Vector2f(1650.0f - animOffset, 620.0f + animOffset));
    sword.setPoint(3, sf::Vector2f(1500.0f - animOffset, 1000.0f));
    
    sword.setFillColor(sf::Color(150, 150, 150));
    sword.setOutlineThickness(2.0f);
    sword.setOutlineColor(sf::Color(50, 50, 50));
    
    window.draw(sword);
}
