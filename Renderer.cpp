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

// bardzo agresywny filtr wyłącznie dla miecza (usuwa fioletowe wygładzenia krawędzi)
bool isMagentaBackgroundAggressive(const sf::Color& c) {
    if (c.a == 0) return true;
    if (c.r > 240 && c.g < 20 && c.b > 240) return true;
    
    // usuwa wszystkie fioletowawe odcienie (fringe)
    if (c.r > c.g + 15 && c.b > c.g + 15) {
        if (std::abs(c.r - c.b) < 60) return true;
    }
    return false;
}

// pomocnicza funkcja do sprawdzania czy kolor to magentowe tło
bool isMagentaBackground(const sf::Color& c, bool isZjawa = false, int relX = 250, int relY = 200, int frameIndex = 0) {
    int r = c.r;
    int g = c.g;
    int b = c.b;

    if (isZjawa) {
        // czyste jasne tło magentowe (r>200, b>200) usuwamy zanim zaczniemy chronić aury
        if (r > 200 && b > 200 && g < 50) return true;

        // zjawa: ostateczny zabójca "różowej obwódki" powstałej z antyaliasingu w programie graficznym.
        // ucinamy wszystko co zdradza objawy magenty powyżej poziomu płaszcza (r~50)
        if (r > 75 && b > 75 && std::abs(r - b) < 60 && g < r - 15 && g < b - 15) {
            
            // prawe kule (relx > 450) mają fioletową poświatę (r~132)! chronimy je tylko w klatkach ataku (2 i 3).
            if ((frameIndex == 2 || frameIndex == 3) && relX > 450) {
                return false; // tło (r>200) ucięliśmy u góry! tutaj trafia tylko ciemniejsza poświata (r~132).
            }

            return true; // usuń całą resztę zanieczyszczonej magenty
        }
        
        // zjawa: oczyszczacz "lewego marginesu" (niedomalowane czarne tło na pikselach 0-60)
        if (relX < 60 && g < 80) return true;

        // zjawa: zabójca czarnej pionowej kreski (lewej ramki odessanej z następnej komórki).
        if (relX >= 550 && r < 60 && g < 60 && b < 60) return true;
        
        if (r > 100 && b > 100 && std::abs(r - b) < 40 && g < 80) return true;
    } else {
        // dla goblina i szkieletów
        if (r > g + 10 && b > g + 10) {
            if (std::abs(r - b) < 60) return true;
        }
    }
    
    if (r == 255 && g == 0 && b == 255) return true;
    if (c.r > 240 && c.g < 20 && c.b > 240) return true;
    
    return false;
}

// sprawdza, czy piksel należy do postaci
bool isForegroundPixel(const sf::Color& c, bool isZjawa = false, int relX = 250, int relY = 200, int frameIndex = 0) {
    if (isMagentaBackground(c, isZjawa, relX, relY, frameIndex)) return false;
    if (c.r < 20 && c.g < 20 && c.b < 20) return false;
    return true;
}

// wyodrębnianie klatek ze spritesheeta - nowa wersja adaptacyjna
void Renderer::extractSpriteFrames(int textureIndex, const sf::Image& img) {
    int origW = img.getSize().x;
    int origH = img.getSize().y;
    
    // zdjęcia wrogów (indeksy 0-5) mają dwa rzędy! zdjęcia itemów (6-8) mają jeden rząd!
    int blockH = (textureIndex < 6) ? (origH / 2) : origH; 
    
    // zakładamy, że obraz jest równo podzielony na klatki (jeśli to wróg to 5 klatek, inaczej 1)
    int numFrames = (textureIndex >= 6) ? 1 : 5;
    int frameW = origW / numFrames;
    
    spriteFrames[textureIndex].resize(5); // gra zakłada maksymalnie 5 stanów
    for (int i = 0; i < 5; ++i) {
        int cellIdx = i;
        if (cellIdx >= numFrames) cellIdx = numFrames - 1;
        
        int cellMinX = cellIdx * frameW;
        int cellMaxX = (cellIdx + 1) * frameW - 1;
        if (cellIdx == numFrames - 1) cellMaxX = origW - 1;
        
        // zjawa i wrogówie mają niesymetryczne, niezwykle szerokie klatki (kule magii).
        // magia tak bardzo wychodzi w prawo, że aż wlewa się na lewą stronę następnej klatki!
        // dlatego przesuwamy całe okno skanowania o 30 pikseli w prawo.
        int sMinX = cellMinX;
        int sMaxX = cellMaxX;
        int sMinY = 0;
        int sMaxY = blockH - 1;

        if (textureIndex >= 1 && textureIndex <= 5) {
            sMinX = cellMinX + 30; // omija lewe krwawiące kule (z poprzedniej klatki)
            sMaxX = cellMaxX + 30; // domyślnie pozwalamy na czytanie lekko wychodzących sprite'ów
            
            // inteligencja dla zjawy (textureindex == 3)
            if (textureIndex == 3) {
                // tylko klatka nr 2 ładuje ogromne kule, które wychodzą aż do relx=614! dajemy jej +70 przestrzeni.
                // klatka nr 3 i pozostałe kończą się wewnątrz własnej komórki (551).
                if (i == 2) {
                    sMaxX = cellMaxX + 70;
                } else {
                    sMaxX = cellMaxX;
                }
            }
            
            if (sMaxX >= origW) sMaxX = origW - 1;
            sMinY = 40;
            sMaxY = blockH - 1; // przestajemy ucinać 40 pikseli z dołu, żeby nie obcinać stóp npc!
        } else {
            if (sMinX > sMaxX) { sMinX = cellMinX; sMaxX = cellMaxX; }
        }
        
        int yOffset = 0;
        if (textureIndex >= 1 && textureIndex <= 5) {
            yOffset = 0; // używamy pierwszego rzędu dla wszystkich animacji
        }

        int minX = sMaxX;
        int maxX = sMinX;
        int minY = sMaxY;
        int maxY = sMinY;

        // szukamy granic (bounding box) omijając sąsiadów i górny/dolny brud!
        for (int x = sMinX; x <= sMaxX; ++x) {
            for (int y = sMinY; y <= sMaxY; ++y) {
                sf::Color c = img.getPixel(x, y + yOffset);
                int relX = x - cellMinX;
                int relY = y;
                bool isForeground = false;
                if (textureIndex == 3) {
                    isForeground = isForegroundPixel(c, true, relX, relY, i);
                } else {
                    isForeground = isForegroundPixel(c);
                }
                if (isForeground) {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
        
        // zabezpieczenie przed pustą komórką
        if (minX > maxX || minY > maxY) {
            minX = cellMinX; maxX = cellMaxX;
            minY = 0; maxY = blockH - 1;
        }
        
        int w = maxX - minX + 1;
        int h = maxY - minY + 1;
        
        spriteFrames[textureIndex][i].width = w;
        spriteFrames[textureIndex][i].height = h;
        spriteFrames[textureIndex][i].blockH = blockH;
        spriteFrames[textureIndex][i].xOffset = minX - cellMinX;
        spriteFrames[textureIndex][i].bottomOffset = blockH - (minY + h);
        spriteFrames[textureIndex][i].pixels.resize(w * h, sf::Color(0, 0, 0, 0));
        
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                sf::Color c = img.getPixel(minX + x, minY + y + yOffset);
                int absX = minX + x;
                int relX = absX - (i * frameW);
                int relY = minY + y;

                if (isMagentaBackground(c, textureIndex == 3, relX, relY, i)) {
                    spriteFrames[textureIndex][i].pixels[y * w + x] = sf::Color(0, 0, 0, 0);
                } else {
                    spriteFrames[textureIndex][i].pixels[y * w + x] = c;
                }
            }
        }
        
        // erozja: uniwersalne usuwanie 1-pikselowej cienkiej obwódki po magentowym tle z krawędzi każdego potwóra!
        if (textureIndex >= 1 && textureIndex <= 5 && w > 2 && h > 2) {
            std::vector<sf::Color> newPixels = spriteFrames[textureIndex][i].pixels;
            for (int py = 1; py < h - 1; ++py) {
                for (int px = 1; px < w - 1; ++px) {
                    sf::Color c = spriteFrames[textureIndex][i].pixels[py * w + px];
                    // złap jakikolwiek gradient obwódki antyaliasingu o zabarwieniu magentowym (r i b wyższe niż g)
                    if (c.a != 0 && c.r > c.g + 5 && c.b > c.g + 5 && std::abs((int)c.r - (int)c.b) < 40) {
                        // sprawdź czy jest pikselem krawędziowym (dotyka przezroczystości)
                        bool touchesTransparent = 
                            spriteFrames[textureIndex][i].pixels[(py - 1) * w + px].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[(py + 1) * w + px].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[py * w + (px - 1)].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[py * w + (px + 1)].a == 0;
                            
                        // odetnij zanieczyszczony piksel obwódki!
                        if (touchesTransparent) newPixels[py * w + px] = sf::Color(0,0,0,0);
                    }
                }
            }
            spriteFrames[textureIndex][i].pixels = newPixels;
        }
    }
    
    // debug: zapisz wszystkie klatki zjawy jako duży poziomy pasek!
    if (textureIndex == 3) {
        int totalW = 0;
        int maxH = 0;
        for (int i=0; i<numFrames; ++i) {
            totalW += spriteFrames[textureIndex][i].width;
            if (spriteFrames[textureIndex][i].height > maxH) maxH = spriteFrames[textureIndex][i].height;
        }
        sf::Image dbg;
        dbg.create(totalW > 0 ? totalW : 100, maxH > 0 ? maxH : 100, sf::Color::Transparent);
        int curX = 0;
        for (int i=0; i<numFrames; ++i) {
            int w = spriteFrames[textureIndex][i].width;
            int h = spriteFrames[textureIndex][i].height;
            for(int y = 0; y < h; ++y) {
                for(int x = 0; x < w; ++x) {
                    dbg.setPixel(curX + x, y, spriteFrames[textureIndex][i].pixels[y * w + x]);
                }
            }
            curX += w;
        }
        dbg.saveToFile("DEBUG_ALL_ZJAWA.png");
    }
}

// pomocnicza funkcja do znajdowania ścieżki
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
    
    // wczytanie broni gracza z bezbłędnym nałożeniem przezroczystości
    sf::Image weaponImg;
    playerWeaponFrames.clear();
    if (weaponImg.loadFromFile(getTexPath("textures/miecz-2.png"))) {
        int w = weaponImg.getSize().x;
        int h = weaponImg.getSize().y;
        
        std::vector<bool> colHasPixels(w, false);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (isMagentaBackgroundAggressive(weaponImg.getPixel(x, y))) {
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
    
    sf::Image hudImg;
    if (hudImg.loadFromFile(getTexPath("textures/hud-2.png"))) {
        int w = hudImg.getSize().x;
        int h = hudImg.getSize().y;
        
        // znalezienie granic (crop) by dopasować się idealnie do widocznego hud-a
        unsigned int minX = w, minY = h;
        unsigned int maxX = 0, maxY = 0;
        for (int y = h / 2; y < h; ++y) { // skanujemy tylko dolną połowę obrazu, by zignorować wklejony zrzut ekranu ide z góry
            for (int x = 0; x < w; ++x) {
                if (hudImg.getPixel(x, y).a > 0) { // uwzględnia tylko nieprzezroczyste piksele
                    if ((unsigned int)x < minX) minX = x;
                    if ((unsigned int)y < minY) minY = y;
                    if ((unsigned int)x > maxX) maxX = x;
                    if ((unsigned int)y > maxY) maxY = y;
                }
            }
        }
        
        if (minX <= maxX && minY <= maxY) {
            sf::Image cropped;
            unsigned int cropW = maxX - minX + 1;
            unsigned int cropH = maxY - minY + 1;
            cropped.create(cropW, cropH, sf::Color(0,0,0,0));
            cropped.copy(hudImg, 0, 0, sf::IntRect(minX, minY, cropW, cropH), true);
            hudTexture.loadFromImage(cropped);
        } else {
            hudTexture.loadFromImage(hudImg);
        }
        hudSprite.setTexture(hudTexture);
    }
    
    hudFont.loadFromFile(getTexPath("textures/font.ttf"));
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

// prosty algorytm sortujący wrogów i przedmioty od najdalszego do najbliższego
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

            setPixel(x, y, mapTextures[6][texWidth * ceilTexY + ceilTexX]);
        }

        // rysowanie ściany
        for (int y = drawStart; y < drawEnd; y++) {
            int texY = (int)texPos & (texHeight - 1);
            texPos += step;
            sf::Color color = mapTextures[texNum][texWidth * texY + texX];

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

            setPixel(x, y, mapTextures[5][texWidth * floorTexY + floorTexX]);
        }
        
        // zapisanie dystansu do ściany dla danego x ekranu na potrżeby sortowania dogłębnego sprite'ów
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
        
        // macierz odwrotnej kamery żeby przetłumaczyć to na piksele ekranowe
        double invdet = 1.0 / (planex * diry - dirx * planey);
        double transformx = invdet * (diry * spritex - dirx * spritey);
        double transformy = invdet * (-planey * spritex + planex * spritey);
        
        int spritescreenx = int((bufferwidth / 2) * (1 + transformx / transformy));
        
        // wymiary spritea opierają się o transformy żeby nie było efektu rybiego oka
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
                    
                    // pomijamy kolor jeśli jego alfa jest równa zero albo używamy koloru czarnego jako tło zakładamy że czysty czarny to brak piksela
                    if(color.r != 0 || color.g != 0 || color.b != 0) {
                        setpixel(stripe, y, color);
                    }
                }
            }
        }
    }}*/
    // tworzymy strukturę pomocniczą tylko na potrżeby tej funkcji
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
        allSprites.push_back({ enemy->getX(), enemy->getY(), enemy->getTexture(), static_cast<int>(enemy->getState()), enemy->isMirrored() });
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
    
    //  wspólne sortowanie
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


        // jedna wspólna pętla rysująca metodą lodeva

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
            int textureIndex = allSprites[currentIdx].texture;

            // obliczamy fizyczną szerokość sprite'a na ekranie!
            // uwaga: ponieważ zjawa i wrogówie mają klatki w dwóch rzędach (więc ich tekstury są wysokie na dysku),
            // musimy manualnie pomnożyć szerokość przez 2, aby przywrócić im proporcje kwadratu (lub szerokiego płaszcza).
            int spriteWidth = abs(int(bufferHeight / (transformY))) * frame.width / frame.height;
            if (textureIndex >= 6 || textureIndex == 3) {
                // dla tekstur z dwoma rzędami klatek, mnożymy szerokość x2, aby odzyskać właściwe proporcje
                spriteWidth = spriteWidth * 2;
            }

            int spriteHeight = std::abs(int(bufferHeight / transformY));
            double scaleModifier = 1.0;
            if (allSprites[currentIdx].texture == 6) { // orb
                scaleModifier = 0.3; // mniejsza skala dla orba (30% bazowego rozmiaru)
            } else if (allSprites[currentIdx].texture == 8) { // potka
                scaleModifier = 0.4; // zmniejszamy miksturę leczenia (40%)
            }
            double scale = ((double)spriteHeight / frame.blockH) * scaleModifier;
            
            int drawHeight = frame.height * scale;
            int drawWidth = frame.width * scale;

            // pozycjonowanie w pionie: dół sprite'a ma dotykać podłogi pomniejszonej o bottomoffset
            int floorY = spriteHeight / 2 + bufferHeight / 2;
            int unclampedDrawEndY = floorY - frame.bottomOffset * scale;
            int drawEndY = unclampedDrawEndY;
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
                        int trueDrawStartY = unclampedDrawEndY - drawHeight;
                        int texY = int(256 * (y - trueDrawStartY) * frame.height / drawHeight) / 256;

                        if (texX >= 0 && texX < frame.width && texY >= 0 && texY < frame.height) {
                            sf::Color color = frame.pixels[frame.width * texY + texX];

                            if(color.a != 0) {
                                // czerwone wypełnienie (tint) dla wrogów przy otrzymywaniu obrażeń (stan 3 - hurt)
                                if (allSprites[currentIdx].texture < 6 && allSprites[currentIdx].state == 3) {
                                    color.r = static_cast<sf::Uint8>(std::min(255, static_cast<int>(color.r) + 150));
                                    color.g = static_cast<sf::Uint8>(color.g * 0.2);
                                    color.b = static_cast<sf::Uint8>(color.b * 0.2);
                                }
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


// rysowanie bazowego interfejsu z przezroczystym obrazkiem i działającym tłem (krwią)
void Renderer::drawHud(sf::RenderWindow& window, const Player& player) {
    if (hudTexture.getSize().x > 0) {
        sf::Vector2u texSize = hudTexture.getSize();
        
        // skalujemy proporcjonalnie, żeby kula była faktycznie kulą, a nie spłaszczoną elipsą!
        float scaleY = (screenHeight * 0.25f) / texSize.y;
        float scaleX = scaleY; // utrzymujemy proporcje!
        
        float hudW = texSize.x * scaleX;
        float hudH = texSize.y * scaleY;
        float hudY = screenHeight - hudH;
        float hudX = (screenWidth - hudW) / 2.0f; // centrujemy hud na dole ekranu

        hudSprite.setScale(scaleX, scaleY);
        hudSprite.setPosition(hudX, hudY);

        // parametry idealnie wpasowujące się w prawdziwy otwór w hud
        float radX = (texSize.x * 0.0747f) * scaleX;
        float radY = (texSize.y * 0.2868f) * scaleY;
        float orbX = hudX + (texSize.x * 0.1226f) * scaleX;
        float orbY = hudY + (texSize.y * 0.5698f) * scaleY;

        // 1. czarne tło tylko pod kulę (aby widać było próżnię przy braku zdrowia)
        sf::CircleShape bgCircle(radX);
        bgCircle.setScale(1.0f, radY / radX);
        bgCircle.setFillColor(sf::Color(10, 10, 10)); // ciemne
        bgCircle.setOrigin(radX, radX);
        bgCircle.setPosition(orbX, orbY);
        window.draw(bgCircle);

        // 2. czerwony płyn życia
        float hpPercent = std::max(0.0f, std::min(100.0f, static_cast<float>(player.getHp()))) / 100.0f;
        sf::CircleShape fluidCircle(radX);
        fluidCircle.setScale(1.0f, radY / radX);
        fluidCircle.setFillColor(sf::Color(200, 20, 20));
        fluidCircle.setOrigin(radX, radX);
        fluidCircle.setPosition(orbX, orbY);
        window.draw(fluidCircle);
        
        // maska obcinająca płyn od góry (symuluje opadanie cieczy)
        float emptyHeight = (radY * 2.0f) * (1.0f - hpPercent);
        if (emptyHeight > 0) {
            sf::RectangleShape mask(sf::Vector2f(radX * 2.5f, emptyHeight));
            mask.setFillColor(sf::Color(10, 10, 10)); // takie samo jak bgcircle
            mask.setOrigin(mask.getSize().x / 2.0f, 0);
            mask.setPosition(orbX, orbY - radY);
            window.draw(mask);
        }

        // 3. właściwa ramka hud na wierzch (przezroczysta dziura na kulę, własne czarne sloty)
        window.draw(hudSprite);

        // 4. efekt przezroczystego szkła na wierzch kuli (dodaje realizmu)
        sf::CircleShape glass(radX);
        glass.setScale(1.0f, radY / radX);
        glass.setFillColor(sf::Color(255, 255, 255, 25)); // delikatna biel z alphą (szkło)
        glass.setOrigin(radX, radX);
        glass.setPosition(orbX, orbY);
        window.draw(glass);
    }
}

// rysowanie aktywnej broni zależnie od stanu gracza
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
    
    // miecz powiększony do 1.0f
    float scale = 1.0f;
    // odbijamy lustrzanie tylko pierwszą klatkę, skalujemy do sensownego rozmiaru
    if (frameIndex == 0 && isSpriteSheet) {
        playerWeaponSprite.setScale(-scale, scale); 
    } else {
        playerWeaponSprite.setScale(scale, scale);
    }
    
    // miecz przyklejony do prawej krawędzi ekranu
    float swordX = screenWidth - (currentFrame.width / 2.0f * scale);
    float hudHeight = screenHeight * 0.25f;
    
    // opuszczamy miecz niżej, tak by chował się za hud-em
    playerWeaponSprite.setPosition(swordX - animOffsetX, screenHeight - hudHeight + 420.0f + animOffsetY);
    
    // rotacja bazowa dla postawy idle i na to nakładana animacja
    float baseRotation = 0.0f; // 0 zamiast -15 by miecz był bardziej pionowo i widoczny w całości
    playerWeaponSprite.setRotation(baseRotation + rotation);
    
    window.draw(playerWeaponSprite);
}
