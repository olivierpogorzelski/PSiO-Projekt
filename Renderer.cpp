#include "Renderer.hpp"
#include <cmath>
#include <algorithm>

Renderer::Renderer()
    : mapTextures(7, std::vector<sf::Color>(texWidth * texHeight)),
      spriteFrames(9),
      screenPixels(bufferWidth * bufferHeight * 4, 0) 
{
    loadTextures();

    screenTexture.create(bufferWidth, bufferHeight);
    screenTexture.setSmooth(false);

    screenSprite.setTexture(screenTexture);
    screenSprite.setScale((float)screenWidth / bufferWidth, (float)screenHeight / bufferHeight);
}

// pomocnicza funkcja do skalowania map tekstur
std::vector<sf::Color> Renderer::scaleImage(const sf::Image& img, int targetW, int targetH) {
    std::vector<sf::Color> result(targetW * targetH);
    int origW = img.getSize().x;
    int origH = img.getSize().y;
    for (int y = 0; y < targetH; ++y) {
        for (int x = 0; x < targetW; ++x) {
            int srcX = x * origW / targetW;
            int srcY = y * origH / targetH;
            result[y * targetW + x] = img.getPixel(srcX, srcY);
        }
    }
    return result;
}

// pomocnicza funkcja do sprawdzania czy kolor to magentowe tło (wraz z wygładzonymi krawędziami)
bool isMagentaBackground(const sf::Color& c) {
    if (c.a == 0) return true;
    if (c.r == 255 && c.g == 0 && c.b == 255) return true;
    
    // usuwanie antyaliasingu magenty (tzw. fringe), który tworzy fioletowe obwódki na krawędziach
    // zwykle na granicy tła czerwony i niebieski znacznie przewyžszają zielony.
    if (c.r > c.g + 15 && c.b > c.g + 15) {
        // skoro magenta to r == b, to krawędzie antyaliasingowe tež powinny mieć zbližone wartości r i b
        if (std::abs(c.r - c.b) < 40) return true;
    }
    return false;
}

// sprawdza, czy piksel naležy do postaci (nie jest tłem ani czarną linią siatki)
bool isForegroundPixel(const sf::Color& c) {
    if (isMagentaBackground(c)) return false;
    // ciemne piksele (czarne linie siatki) ignorujemy podczas szukania granic!
    // prawdziwa postać i tak będzie miała w tej samej kolumnie jaśniejsze piksele.
    if (c.r < 20 && c.g < 20 && c.b < 20) return false;
    return true;
}

// wyodrębnianie klatek ze spritesheeta - nowa wersja adaptacyjna
void Renderer::extractSpriteFrames(int textureIndex, const sf::Image& img) {
    int origW = img.getSize().x;
    int origH = img.getSize().y;
    
    // zdjĘcia wrogÓw (indeksy 0-5) majĄ dwa rzĘdy! zdjĘcia itemÓw (6-8) majĄ jeden rzĄd!
    int blockH = (textureIndex < 6) ? (origH / 2) : origH; 
    
    // zakładamy, že obraz jest równo podzielony na klatki (jeśli to wróg to 5 klatek, inaczej 1)
    int numFrames = (textureIndex >= 6) ? 1 : 5;
    int frameW = origW / numFrames;
    
    spriteFrames[textureIndex].resize(5); // gra zakłada maksymalnie 5 stanów
    for (int i = 0; i < 5; ++i) {
        int cellIdx = i;
        if (cellIdx >= numFrames) cellIdx = numFrames - 1;
        
        int cellMinX = cellIdx * frameW;
        int cellMaxX = (cellIdx + 1) * frameW - 1;
        if (cellIdx == numFrames - 1) cellMaxX = origW - 1;
        
        // potĘŻny margines na śmieciowe ramki siatki! wrogowie dostają margines 6 pikseli z každej strony.
        int pad = (textureIndex >= 6) ? 0 : 6; 
        int sMinX = cellMinX + pad;
        int sMaxX = cellMaxX - pad;
        if (sMinX > sMaxX) { sMinX = cellMinX; sMaxX = cellMaxX; } // fallback dla bardzo małych tekstur
        
        int minX = sMaxX;
        int maxX = sMinX;
        int minY = blockH;
        int maxY = 0;
        
        // szukamy granic (bounding box) dla rzeczywistych pikseli postaci wewnątrz przyciętej komórki
        for (int x = sMinX; x <= sMaxX; ++x) {
            for (int y = pad; y < blockH - pad; ++y) {
                if (isForegroundPixel(img.getPixel(x, y))) {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
        
        if (minY > maxY || minX > maxX) {
            // klatka pusta! 
            if (i > 0) {
                spriteFrames[textureIndex][i] = spriteFrames[textureIndex][i-1];
                continue;
            } else {
                // jeśli pierwsza klatka jest pusta (np. bardzo ciemny pocisk), wymuszamy pełny rozmiar
                minX = cellMinX;
                maxX = cellMaxX;
                minY = 0;
                maxY = blockH - 1;
            }
        }
        
        int w = maxX - minX + 1;
        int h = maxY - minY + 1;
        
        spriteFrames[textureIndex][i].width = w;
        spriteFrames[textureIndex][i].height = h;
        spriteFrames[textureIndex][i].pixels.resize(w * h);
        spriteFrames[textureIndex][i].bottomOffset = blockH - maxY;
        
        // obliczamy offset
        float expectedCenter = cellMinX + frameW / 2.0f;
        if (textureIndex >= 6) expectedCenter = origW / 2.0f;
        
        spriteFrames[textureIndex][i].xOffset = (minX + w / 2) - expectedCenter;
        spriteFrames[textureIndex][i].blockH = blockH;
        
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                sf::Color c = img.getPixel(minX + x, minY + y);
                if (isMagentaBackground(c)) c = sf::Color(0, 0, 0, 0); 
                spriteFrames[textureIndex][i].pixels[y * w + x] = c;
            }
        }
    }
}

// pomocnicza funkcja do znajdowania ściežki
std::string getTexPath(const std::string& filename) {
    std::vector<std::string> paths = {
        filename,
        "../" + filename,
        "../../" + filename,
        "../../../" + filename
    };
    for (const auto& p : paths) {
        sf::Image test;
        if (test.loadFromFile(p)) return p;
    }
    return filename; // fallback
}

// ładowanie tekstur z plików
void Renderer::loadTextures() {
    sf::Image wallImg, floorImg, ceilingImg;
    if (wallImg.loadFromFile(getTexPath("textures/wall.png"))) {
        auto tex = scaleImage(wallImg, texWidth, texHeight);
        for(int i = 0; i < 5; i++) mapTextures[i] = tex;
    }
    if (floorImg.loadFromFile(getTexPath("textures/floor.png"))) {
        mapTextures[5] = scaleImage(floorImg, texWidth, texHeight);
    }
    if (ceilingImg.loadFromFile(getTexPath("textures/ceiling.png"))) {
        mapTextures[6] = scaleImage(ceilingImg, texWidth, texHeight);
    }
    
    std::vector<std::string> spriteFiles = {
        "textures/duzydemon.png",     // 0
        "textures/goblin.png",        // 1
        "textures/malydemon.png",     // 2
        "textures/zjawa.png",         // 3
        "textures/szkielet-miecz.png",// 4
        "textures/szkielet-luk.png",  // 5
        "textures/orb.png",           // 6
        "textures/strzala.png",       // 7
        "textures/potka.png"          // 8
    };
    
    for (size_t i = 0; i < spriteFiles.size(); ++i) {
        sf::Image img;
        if (img.loadFromFile(getTexPath(spriteFiles[i]))) {
            extractSpriteFrames(i, img);
        } else {
            spriteFrames[i].resize(5);
            for(int j=0; j<5; j++) {
                spriteFrames[i][j].width = 1;
                spriteFrames[i][j].height = 1;
                spriteFrames[i][j].pixels = {sf::Color::Transparent};
            }
        }
    }
    
    // wczytanie broni gracza z bezbłędnym nałoženiem przezroczystości
    sf::Image weaponImg;
    playerWeaponFrames.clear();
    if (weaponImg.loadFromFile(getTexPath("textures/miecz-2.png"))) {
        int w = weaponImg.getSize().x;
        int h = weaponImg.getSize().y;
        
        std::vector<bool> colHasPixels(w, false);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (isMagentaBackground(weaponImg.getPixel(x, y))) {
                    weaponImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
                } else {
                    colHasPixels[x] = true;
                }
            }
        }
        
        // skanowanie klastrów pikseli (klatek broni) w osi y (rzędy najpierw)
        std::vector<sf::IntRect> rows;
        bool inCluster = false;
        int startY = 0;
        int gap = 0;
        for (int y = 0; y < h; ++y) {
            bool rowHasPixels = false;
            for (int x = 0; x < w; ++x) {
                if (weaponImg.getPixel(x, y).a > 0) { rowHasPixels = true; break; }
            }
            if (rowHasPixels) {
                if (!inCluster) { inCluster = true; startY = y; }
                gap = 0;
            } else if (inCluster) {
                gap++;
                if (gap > 10) { // tolerancja dla małych przerw
                    rows.push_back(sf::IntRect(0, startY, w, y - gap - startY));
                    inCluster = false;
                }
            }
        }
        if (inCluster) {
            rows.push_back(sf::IntRect(0, startY, w, h - gap - startY));
        }
        
        // skanowanie wewnątrz rzędów w osi x (aby czytać klatki od lewej do prawej, z góry na dół)
        for (auto& row : rows) {
            bool inX = false;
            int startX = 0;
            int gapX = 0;
            for (int x = 0; x < w; ++x) {
                bool colHasPixels = false;
                for (int y = row.top; y < row.top + row.height; ++y) {
                    if (weaponImg.getPixel(x, y).a > 0) {
                        colHasPixels = true; break;
                    }
                }
                if (colHasPixels) {
                    if (!inX) { inX = true; startX = x; }
                    gapX = 0;
                } else if (inX) {
                    gapX++;
                    if (gapX > 10) { // przerwa w poziomie
                        playerWeaponFrames.push_back(sf::IntRect(startX, row.top, x - gapX - startX, row.height));
                        inX = false;
                    }
                }
            }
            if (inX) {
                playerWeaponFrames.push_back(sf::IntRect(startX, row.top, w - gapX - startX, row.height));
            }
        }
        
        // fallback w przypadku pustego pliku
        if (playerWeaponFrames.empty()) {
            playerWeaponFrames.push_back(sf::IntRect(0, 0, w, h));
        }
        
        playerWeaponTexture.loadFromImage(weaponImg);
        playerWeaponSprite.setTexture(playerWeaponTexture);
        playerWeaponSprite.setScale(1.0f, 1.0f);
    }
}

// ustawia pojedynczy piksel na buforze uwzględniając format rgba
void Renderer::setPixel(int x, int y, sf::Color color) {
    if (x < 0 || x >= bufferWidth || y < 0 || y >= bufferHeight) return;
    int index = (y * bufferWidth + x) * 4;
    screenPixels[index]     = color.r;
    screenPixels[index + 1] = color.g;
    screenPixels[index + 2] = color.b;
    screenPixels[index + 3] = color.a;
}

// prosty algorytm sortujący wrogów i przedmioty od najdalszego do najbližszego
void Renderer::sortSprites(std::vector<int>& order, std::vector<double>& dist) {
    std::vector<std::pair<double, int>> sprites(order.size());
    for(size_t i = 0; i < order.size(); i++) {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }
    std::sort(sprites.begin(), sprites.end());
    // odwracamy žeby rysować najpierw to co z tyłu
    for(size_t i = 0; i < order.size(); i++) {
        dist[i] = sprites[order.size() - i - 1].first;
        order[i] = sprites[order.size() - i - 1].second;
    }
}

// główne renderowanie
void Renderer::render(sf::RenderWindow& window, const Player& player, const Map& map) {
    // czyszczenie bufora na czarno žeby uniknąć śmieci
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

    // pętla po každej pionowej kolumnie ekranu
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

            setPixel(x, y, mapTextures[6][texWidth * ceilTexY + ceilTexX]);
        }

        // rysowanie ściany
        for (int y = drawStart; y < drawEnd; y++) {
            int texY = (int)texPos & (texHeight - 1);
            texPos += step;
            sf::Color color = mapTextures[texNum][texWidth * texY + texX];

            // przyciemnienie ścian žeby dać iluzję cienia
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

            setPixel(x, y, mapTextures[5][texWidth * floorTexY + floorTexX]);
        }
        
        // zapisanie dystansu do ściany dla danego x ekranu na potrzeby sortowania dogłębnego sprite'ów
        zBuffer[x] = perpWallDist;
    }

    /*// rysowanie wrogów metodą lodeva
    const auto& enemies = map.getenemies();
    size_t numenemies = enemies.size();
    if (numenemies > 0){
    std::vector<int> enemyorder(numenemies);
    std::vector<double> enemydistance(numenemies);
    
    for(size_t i = 0; i < numenemies; i++) {
        enemyorder[i] = i;
        enemydistance[i] = ((posx - enemies[i].getx()) * (posx - enemies[i].getx()) + (posy - enemies[i].gety()) * (posy - enemies[i].gety()));
    }
    
    sortsprites(enemyorder, enemydistance);
    
    for(size_t i = 0; i < numenemies; i++) {
        double spritex = enemies[enemyorder[i]].getx() - posx;
        double spritey = enemies[enemyorder[i]].gety() - posy;
        
        // macierz odwrotnej kamery žeby przetłumaczyć to na piksele ekranowe
        double invdet = 1.0 / (planex * diry - dirx * planey);
        double transformx = invdet * (diry * spritex - dirx * spritey);
        double transformy = invdet * (-planey * spritex + planex * spritey);
        
        int spritescreenx = int((bufferwidth / 2) * (1 + transformx / transformy));
        
        // wymiary spritea opierają się o transformy žeby nie było efektu rybiego oka
        int spriteheight = std::abs(int(bufferheight / transformy));
        int drawstarty = -spriteheight / 2 + bufferheight / 2;
        if(drawstarty < 0) drawstarty = 0;
        int drawendy = spriteheight / 2 + bufferheight / 2;
        if(drawendy >= bufferheight) drawendy = bufferheight - 1;
        
        int spritewidth = std::abs(int(bufferheight / transformy));
        int drawstartx = -spritewidth / 2 + spritescreenx;
        if(drawstartx < 0) drawstartx = 0;
        int drawendx = spritewidth / 2 + spritescreenx;
        if(drawendx >= bufferwidth) drawendx = bufferwidth - 1;
        
        for(int stripe = drawstartx; stripe < drawendx; stripe++) {
            int texx = int(256 * (stripe - (-spritewidth / 2 + spritescreenx)) * texwidth / spritewidth) / 256;
            
            // czy mieści się przed kamerą na ekranie oraz czy nie chowa się za ścianą
            if(transformy > 0 && stripe > 0 && stripe < bufferwidth && transformy < zbuffer[stripe]) {
                for(int y = drawstarty; y < drawendy; y++) {
                    int d = (y) * 256 - bufferheight * 128 + spriteheight * 128;
                    int texy = ((d * texheight) / spriteheight) / 256;
                    
                    sf::color color = textures[enemies[enemyorder[i]].gettexture()][texwidth * texy + texx];
                    
                    // pomijamy kolor jeśli jego alfa jest równa zero albo užywamy koloru czarnego jako tło zakładamy že czysty czarny to brak piksela
                    if(color.r != 0 || color.g != 0 || color.b != 0) {
                        setpixel(stripe, y, color);
                    }
                }
            }
        }
    }}*/
    // tworzymy strukturę pomocniczą tylko na potrzeby tej funkcji
    struct TempSprite {
        double x;
        double y;
        int texture;
        int state;
        bool mirrored;
    };
    std::vector<TempSprite> allSprites;

    // kopia wrogów do wspólnej paczki
    const auto& enemies = map.getEnemies();
    for(const auto& enemy : enemies) {
        allSprites.push_back({ enemy.getX(), enemy.getY(), enemy.getTexture(), static_cast<int>(enemy.getState()), enemy.isMirrored() });
    }

    // kopia niepodniesionych przedmiotów do tej samej wspólnej paczki
    const auto& items = map.getItems();
    for(const auto& item : items) {
        if (!item.isPickedUp) {
            allSprites.push_back({ item.x, item.y, item.texture, 0, false });
        }
    }

    // kopia lecących pocisków
    const auto& projectiles = map.getProjectiles();
    for (const auto& proj : projectiles) {
        // pociski latają w klatce 0
        allSprites.push_back({ proj.getX(), proj.getY(), proj.getTexture(), 0, false });
    }
    
    //  wspÓlne sortowanie
    size_t numSprites = allSprites.size();
    if (numSprites > 0) {
        std::vector<int> spriteOrder(numSprites);
        std::vector<double> spriteDistance(numSprites);

        // liczymy dystans dla wszystkiego na raz
        for(size_t i = 0; i < numSprites; i++) {
            spriteOrder[i] = i;
            double dx = posX - allSprites[i].x;
            double dy = posY - allSprites[i].y;
            spriteDistance[i] = (dx * dx + dy * dy);
        }


        sortSprites(spriteOrder, spriteDistance);


        // jedna wspÓlna pĘtla rysujĄca metodĄ lodeva

        for(size_t i = 0; i < numSprites; i++) {
            int currentIdx = spriteOrder[i];
            
            const auto& frame = spriteFrames[allSprites[currentIdx].texture][allSprites[currentIdx].state];
            if (frame.width <= 1) continue;

            double spriteX = allSprites[currentIdx].x - posX;
            double spriteY = allSprites[currentIdx].y - posY;

            double invDet = 1.0 / (planeX * dirY - dirX * planeY);
            double transformX = invDet * (dirY * spriteX - dirX * spriteY);
            double transformY = invDet * (-planeY * spriteX + planeX * spriteY);

            int spriteScreenX = int((bufferWidth / 2) * (1 + transformX / transformY));

            int spriteHeight = std::abs(int(bufferHeight / transformY)); // to odpowiada frame.blockh
            double scale = (double)spriteHeight / frame.blockH;
            
            int drawHeight = frame.height * scale;
            int drawWidth = frame.width * scale;

            // pozycjonowanie w pionie: dół sprite'a ma dotykać podłogi pomniejszonej o bottomoffset
            int floorY = spriteHeight / 2 + bufferHeight / 2;
            int drawEndY = floorY - frame.bottomOffset * scale;
            int drawStartY = drawEndY - drawHeight;
            
            if(drawStartY < 0) drawStartY = 0;
            if(drawEndY >= bufferHeight) drawEndY = bufferHeight - 1;

            // pozycjonowanie w poziomie: uwzględniamy xoffset
            int drawCenterX = spriteScreenX + frame.xOffset * scale;
            int drawStartX = drawCenterX - drawWidth / 2;
            int drawEndX = drawStartX + drawWidth;
            
            if(drawStartX < 0) drawStartX = 0;
            if(drawEndX >= bufferWidth) drawEndX = bufferWidth - 1;

            for(int stripe = drawStartX; stripe < drawEndX; stripe++) {
                int texX;
                if (allSprites[currentIdx].mirrored) {
                    texX = int(256 * ((drawCenterX + drawWidth / 2) - stripe - 1) * frame.width / drawWidth) / 256;
                } else {
                    texX = int(256 * (stripe - (drawCenterX - drawWidth / 2)) * frame.width / drawWidth) / 256;
                }

                if(transformY > 0 && stripe > 0 && stripe < bufferWidth && transformY < zBuffer[stripe]) {
                    for(int y = drawStartY; y < drawEndY; y++) {
                        // poprawny texy oparty na drawstarty bazowym (bez przycięcia)
                        int trueDrawStartY = drawEndY - drawHeight;
                        int texY = int(256 * (y - trueDrawStartY) * frame.height / drawHeight) / 256;

                        if (texX >= 0 && texX < frame.width && texY >= 0 && texY < frame.height) {
                            sf::Color color = frame.pixels[frame.width * texY + texX];

                            if(color.a != 0) {
                                setPixel(stripe, y, color);
                            }
                        }
                    }
                }
            }
        }
    }
    screenTexture.update(screenPixels.data());
    window.draw(screenSprite);
    drawWeapon(window, player);
    drawHud(window,player);
}


// rysowanie bazowego interfejsu
void Renderer::drawHud(sf::RenderWindow& window, const Player& player) {
    sf::RectangleShape hudBar(sf::Vector2f(1920.0f, 80.0f));
    hudBar.setPosition(0.0f, 1000.0f);
    hudBar.setFillColor(sf::Color(30, 30, 30));
    sf::RectangleShape healthBackground(sf::Vector2f(300.0f, 40.0f));
    healthBackground.setPosition(50.0f, 1020.0f);
    healthBackground.setFillColor(sf::Color(50, 0, 0));
    // rzutujemy hp na float, žeby dzielenie nie ucięło nam ułamków (dzielenie całkowite)
    float maxBarWidth = 300.0f;
    float currentBarWidth = maxBarWidth * (static_cast<float>(player.getHp()) / 100.0f);
    // zabezpieczenie: pasek nie može mieć ujemnej szerokości
    if (currentBarWidth < 0.0f) currentBarWidth = 0.0f;
    sf::RectangleShape healthBar(sf::Vector2f(currentBarWidth, 40.0f));
    healthBar.setPosition(50.0f, 1020.0f);
    healthBar.setFillColor(sf::Color(200, 0, 0));
    
    // najpierw rysujemy wielkie szare tło całego interfejsu
    window.draw(hudBar);
    // potem rysujemy ciemnoczerwony podkład paska zdrowia
    window.draw(healthBackground);
    // na samym końcu nakładamy właściwy, jasnoczerwony pasek zdrowia
    window.draw(healthBar);
}

// rysowanie aktywnej broni zaležnie od stanu gracza
void Renderer::drawWeapon(sf::RenderWindow& window, const Player& player) {
    if (playerWeaponTexture.getSize().x == 0 || playerWeaponFrames.empty()) return;

    bool isSpriteSheet = (playerWeaponFrames.size() >= 3); // jeśli ai wygenerowało 3 lub 4 miecze

    double animOffsetX = 0.0;
    double animOffsetY = 0.0;
    double rotation = 0.0;
    int frameIndex = 0; // klatka 0 = idle
    
    if (player.getAttackTimer() > 0.0) {
        double progress = player.getAttackTimer() / 0.3; // od 1.0 (start ataku) w dół do 0.0
        
        if (isSpriteSheet) {
            // wybór klatki animacji
            if (progress > 0.75) frameIndex = 1;      // zamach
            else if (progress > 0.5) frameIndex = 2; // uderzenie
            else if (progress > 0.2) frameIndex = 3; // kontynuacja
            else frameIndex = 0;                     // powrót
        } else {
            // dynamiczna 3-fazowa animacja cięcia
            if (progress > 0.7) {
                // faza 1: wycofanie miecza (wind-up)
                double p = (1.0 - progress) / 0.3; // 0.0 do 1.0
                animOffsetX = -150.0 * p;
                animOffsetY = 50.0 * p;
                rotation = 20.0 * p;
            } else if (progress > 0.3) {
                // faza 2: błyskawiczne uderzenie (slash)
                double p = (0.7 - progress) / 0.4; // 0.0 do 1.0
                animOffsetX = -150.0 + 600.0 * p;
                animOffsetY = 50.0 - 150.0 * p; 
                rotation = 20.0 - 100.0 * p;
            } else {
                // faza 3: powrót do pozycji startowej (recovery)
                double p = progress / 0.3; // 1.0 do 0.0
                animOffsetX = 450.0 * p;
                animOffsetY = -100.0 * p;
                rotation = -80.0 * p;
            }
        }
    }
    
    if (frameIndex >= playerWeaponFrames.size()) {
        frameIndex = playerWeaponFrames.size() - 1;
    }
    
    sf::IntRect currentFrame = playerWeaponFrames[frameIndex];
    playerWeaponSprite.setTextureRect(currentFrame);
    
    // ustawiamy środek obrotu na środek dolnej krawędzi wyciętej klatki
    playerWeaponSprite.setOrigin(currentFrame.width / 2.0f, currentFrame.height);
    
    // odbijamy lustrzanie tylko pierwszą klatkę
    if (frameIndex == 0 && isSpriteSheet) {
        playerWeaponSprite.setScale(-2.0f, 2.0f); 
    } else {
        playerWeaponSprite.setScale(2.0f, 2.0f);
    }
    
    // miecz z powrotem z prawej strony ekranu (przesunięty o 20 px w prawo)
    playerWeaponSprite.setPosition(1620.0f - animOffsetX, 1080.0f + animOffsetY);
    
    // zwykła rotacja
    playerWeaponSprite.setRotation(rotation);
    
    window.draw(playerWeaponSprite);
}
