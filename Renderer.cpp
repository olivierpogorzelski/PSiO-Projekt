#include "Renderer.hpp"
#include <cmath>
#include <algorithm>

Renderer::Renderer()
    : mapTextures(7, std::vector<sf::Color>(texWidth * texHeight)),
      spriteFrames(11),
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

// bardzo agresywny filtr wylacznie dla miecza (usuwa fioletowe wygladzenia krawedzi)
bool isMagentaBackgroundAggressive(const sf::Color& c) {
    if (c.a == 0) return true;
    if (c.r > 240 && c.g < 20 && c.b > 240) return true;
    
    // usuwa wszystkie fioletowawe odcienie (fringe)
    if (c.r > c.g + 15 && c.b > c.g + 15) {
        if (std::abs(c.r - c.b) < 60) return true;
    }
    return false;
}

// pomocnicza funkcja do sprawdzania czy kolor to magentowe tlo
bool isMagentaBackground(const sf::Color& c, bool isZjawa = false, int relX = 250, int relY = 200, int frameIndex = 0) {
    int r = c.r;
    int g = c.g;
    int b = c.b;

    if (isZjawa) {
        // czyste jasne tlo magentowe (r>200, b>200) usuwamy zanim zaczniemy chronic aury
        if (r > 200 && b > 200 && g < 50) return true;

        // zjawa: ostateczny zabojca "rozowej obwodki" powstalej z antyaliasingu w programie graficznym.
        // ucinamy wszystko co zdradza objawy magenty powyzej poziomu plaszcza (r~50)
        if (r > 75 && b > 75 && std::abs(r - b) < 60 && g < r - 15 && g < b - 15) {
            
            // prawe kule (relx > 450) maja fioletowa poswiate (r~132)! chronimy je tylko w klatkach ataku (2 i 3).
            if ((frameIndex == 2 || frameIndex == 3) && relX > 450) {
                return false; // tlo (r>200) ucielismy u gory! tutaj trafia tylko ciemniejsza poswiata (r~132).
            }

            return true; // usun cala reszte zanieczyszczonej magenty
        }
        
        // zjawa: oczyszczacz "lewego marginesu" (niedomalowane czarne tlo na pikselach 0-60)
        if (relX < 60 && g < 80) return true;

        // zjawa: zabojca czarnej pionowej kreski (lewej ramki odessanej z nastepnej komorki).
        if (relX >= 550 && r < 60 && g < 60 && b < 60) return true;
        
        if (r > 100 && b > 100 && std::abs(r - b) < 40 && g < 80) return true;
    } else {
        // dla goblina i szkieletow
        if (r > g + 10 && b > g + 10) {
            if (std::abs(r - b) < 60) return true;
        }
    }
    
    if (r == 255 && g == 0 && b == 255) return true;
    if (c.r > 240 && c.g < 20 && c.b > 240) return true;
    
    return false;
}

// sprawdza, czy piksel nalezy do postaci
bool isForegroundPixel(const sf::Color& c, bool isZjawa = false, int relX = 250, int relY = 200, int frameIndex = 0) {
    if (isMagentaBackground(c, isZjawa, relX, relY, frameIndex)) return false;
    if (c.r < 20 && c.g < 20 && c.b < 20) return false;
    return true;
}

// wyodrebnianie klatek ze spritesheeta - nowa wersja adaptacyjna
void Renderer::extractSpriteFrames(int textureIndex, const sf::Image& img) {
    int origW = img.getSize().x;
    int origH = img.getSize().y;
    
    // zdjecia wrogow (indeksy 0-5) maja dwa rzedy! zdjecia itemow (6-8) maja jeden rzad!
    int blockH = (textureIndex < 6) ? (origH / 2) : origH; 
    
    // zakladamy, ze obraz jest rowno podzielony na klatki (jesli to wrog to 5 klatek, inaczej 1)
    int numFrames = (textureIndex >= 6) ? 1 : 5;
    int frameW = origW / numFrames;
    
    spriteFrames[textureIndex].resize(5); // gra zaklada maksymalnie 5 stanow
    for (int i = 0; i < 5; ++i) {
        int cellIdx = i;
        if (cellIdx >= numFrames) cellIdx = numFrames - 1;
        
        int cellMinX = cellIdx * frameW;
        int cellMaxX = (cellIdx + 1) * frameW - 1;
        if (cellIdx == numFrames - 1) cellMaxX = origW - 1;
        
        // zjawa i wrogowie maja niesymetryczne, niezwykle szerokie klatki (kule magii).
        // magia tak bardzo wychodzi w prawo, ze az wlewa sie na lewa strone nastepnej klatki!
        // dlatego przesuwamy cale okno skanowania o 30 pikseli w prawo.
        int sMinX = cellMinX;
        int sMaxX = cellMaxX;
        int sMinY = 0;
        int sMaxY = blockH - 1;

        if (textureIndex >= 1 && textureIndex <= 5) {
            sMinX = cellMinX + 30; // omija lewe krwawiace kule (z poprzedniej klatki)
            sMaxX = cellMaxX + 30; // domyslnie pozwalamy na czytanie lekko wychodzacych sprite'ow
            
            // inteligencja dla zjawy (textureindex == 3)
            if (textureIndex == 3) {
                // tylko klatka nr 2 laduje ogromne kule, ktore wychodza az do relx=614! dajemy jej +70 przestrzeni.
                // klatka nr 3 i pozostale koncza sie wewnatrz wlasnej komorki (551).
                if (i == 2) {
                    sMaxX = cellMaxX + 70;
                } else {
                    sMaxX = cellMaxX;
                }
            }
            
            if (sMaxX >= origW) sMaxX = origW - 1;
            sMinY = 40;
            sMaxY = blockH - 1; // przestajemy ucinac 40 pikseli z dolu, zeby nie obcinac stop npc!
        } else {
            if (sMinX > sMaxX) { sMinX = cellMinX; sMaxX = cellMaxX; }
        }
        
        int yOffset = 0;
        if (textureIndex >= 1 && textureIndex <= 5) {
            yOffset = 0; // uzywamy pierwszego rzedu dla wszystkich animacji
        }

        int minX = sMaxX;
        int maxX = sMinX;
        int minY = sMaxY;
        int maxY = sMinY;

        // szukamy granic (bounding box) omijajac sasiadow i gorny/dolny brud!
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
        
        // --- filtr prozniowy (gap filter) ---
        // usuwa oderwane, smieciowe piksele i krwawiace paski z klatek obok.
        // jesli pomiedzy brzegiem a cialem potwora jest >3 pustych kolumn, brzeg zostaje odciety!
        if (minX <= maxX) {
            int maxGap = 3;
            
            // skanowanie od lewej, by uciac smieci na lewym brzegu (np. pasek zjawy)
            int currentGap = 0;
            int realMinX = minX;
            for (int x = minX; x <= maxX; ++x) {
                bool colHasForeground = false;
                for (int y = sMinY; y <= sMaxY; ++y) {
                    sf::Color c = img.getPixel(x, y + yOffset);
                    int relX = x - cellMinX;
                    if (textureIndex == 3 ? isForegroundPixel(c, true, relX, y, i) : isForegroundPixel(c)) {
                        colHasForeground = true; break;
                    }
                }
                if (!colHasForeground) {
                    currentGap++;
                } else {
                    if (currentGap > maxGap) realMinX = x; // resetujemy lewa krawedz po duzej przepasci
                    currentGap = 0;
                }
            }
            minX = realMinX;

            // skanowanie od prawej, by uciac smieci na prawym brzegu
            currentGap = 0;
            int realMaxX = maxX;
            for (int x = maxX; x >= minX; --x) {
                bool colHasForeground = false;
                for (int y = sMinY; y <= sMaxY; ++y) {
                    sf::Color c = img.getPixel(x, y + yOffset);
                    int relX = x - cellMinX;
                    if (textureIndex == 3 ? isForegroundPixel(c, true, relX, y, i) : isForegroundPixel(c)) {
                        colHasForeground = true; break;
                    }
                }
                if (!colHasForeground) {
                    currentGap++;
                } else {
                    if (currentGap > maxGap) realMaxX = x; // resetujemy prawa krawedz po duzej przepasci
                    currentGap = 0;
                }
            }
            maxX = realMaxX;
        }
        // ------------------------------------
        
        // zabezpieczenie przed pusta komorka
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
        
        // erozja: uniwersalne usuwanie 1-pikselowej cienkiej obwodki po magentowym tle z krawedzi kazdego potwora!
        if (textureIndex >= 1 && textureIndex <= 5 && w > 2 && h > 2) {
            std::vector<sf::Color> newPixels = spriteFrames[textureIndex][i].pixels;
            for (int py = 1; py < h - 1; ++py) {
                for (int px = 1; px < w - 1; ++px) {
                    sf::Color c = spriteFrames[textureIndex][i].pixels[py * w + px];
                    // zlap jakikolwiek gradient obwodki antyaliasingu o zabarwieniu magentowym (r i b wyzsze niz g)
                    if (c.a != 0 && c.r > c.g + 5 && c.b > c.g + 5 && std::abs((int)c.r - (int)c.b) < 40) {
                        // sprawdz czy jest pikselem krawedziowym (dotyka przezroczystosci)
                        bool touchesTransparent = 
                            spriteFrames[textureIndex][i].pixels[(py - 1) * w + px].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[(py + 1) * w + px].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[py * w + (px - 1)].a == 0 ||
                            spriteFrames[textureIndex][i].pixels[py * w + (px + 1)].a == 0;
                            
                        // odetnij zanieczyszczony piksel obwodki!
                        if (touchesTransparent) newPixels[py * w + px] = sf::Color(0,0,0,0);
                    }
                }
            }
            spriteFrames[textureIndex][i].pixels = newPixels;
        }
    }
    
    // debug: zapisz wszystkie klatki zjawy jako duzy poziomy pasek!
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

// pomocnicza funkcja do znajdowania sciezki
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

// ladowanie tekstur z plikow
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
        if (img.loadFromFile(getTexPath(spriteFiles[i]))) extractSpriteFrames(i, img);
        else {
            spriteFrames[i].resize(5);
            for(int j=0; j<5; j++) {
                spriteFrames[i][j].width = 1; spriteFrames[i][j].height = 1;
                spriteFrames[i][j].pixels = {sf::Color::Transparent};
            }
        }
    }
    
    // wczytanie broni gracza z bezblednym nalozeniem przezroczystosci
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
        
        // skanowanie klastrow pikseli (klatek broni) w osi y (rzedy najpierw)
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
                if (gap > 10) { // tolerancja dla malych przerw
                    rows.push_back(sf::IntRect(0, startY, w, y - gap - startY));
                    inCluster = false;
                }
            }
        }
        if (inCluster) {
            rows.push_back(sf::IntRect(0, startY, w, h - gap - startY));
        }
        
        // skanowanie wewnatrz rzedow w osi x
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
                    if (gapX > 10) { 
                        playerWeaponFrames.push_back(sf::IntRect(startX, row.top, x - gapX - startX, row.height));
                        inX = false;
                    }
                }
            }
            if (inX) {
                playerWeaponFrames.push_back(sf::IntRect(startX, row.top, w - gapX - startX, row.height));
            }
        }
        
        if (playerWeaponFrames.empty()) playerWeaponFrames.push_back(sf::IntRect(0, 0, w, h));
        
        playerWeaponTexture.loadFromImage(weaponImg);
        playerWeaponSprite.setTexture(playerWeaponTexture);
        playerWeaponSprite.setScale(1.0f, 1.0f);
    }
    
    // wczytanie i czyszczenie kuszy (z tym samym algorytmem!)
    crossbowFrames.clear();
    sf::Image crossbowImg;
    if (crossbowImg.loadFromFile(getTexPath("textures/kusza.png"))) {
        int w = crossbowImg.getSize().x;
        int h = crossbowImg.getSize().y;
        
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (isMagentaBackgroundAggressive(crossbowImg.getPixel(x, y))) {
                    crossbowImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
                }
            }
        }
        
        std::vector<sf::IntRect> rows;
        bool inCluster = false;
        int startY = 0;
        int gap = 0;
        for (int y = 0; y < h; ++y) {
            bool rowHasPixels = false;
            for (int x = 0; x < w; ++x) {
                if (crossbowImg.getPixel(x, y).a > 0) { rowHasPixels = true; break; }
            }
            if (rowHasPixels) {
                if (!inCluster) { inCluster = true; startY = y; }
                gap = 0;
            } else if (inCluster) {
                gap++;
                if (gap > 10) {
                    rows.push_back(sf::IntRect(0, startY, w, y - gap - startY));
                    inCluster = false;
                }
            }
        }
        if (inCluster) rows.push_back(sf::IntRect(0, startY, w, h - gap - startY));
        
        for (auto& row : rows) {
            bool inX = false;
            int startX = 0;
            int gapX = 0;
            for (int x = 0; x < w; ++x) {
                bool colHasPixels = false;
                for (int y = row.top; y < row.top + row.height; ++y) {
                    if (crossbowImg.getPixel(x, y).a > 0) {
                        colHasPixels = true; break;
                    }
                }
                if (colHasPixels) {
                    if (!inX) { inX = true; startX = x; }
                    gapX = 0;
                } else if (inX) {
                    gapX++;
                    if (gapX > 10) { 
                        crossbowFrames.push_back(sf::IntRect(startX, row.top, x - gapX - startX, row.height));
                        inX = false;
                    }
                }
            }
            if (inX) crossbowFrames.push_back(sf::IntRect(startX, row.top, w - gapX - startX, row.height));
        }
        
        crossbowTexture.loadFromImage(crossbowImg);
        crossbowSprite.setTexture(crossbowTexture);
    }
    
    // wczytanie hud z oryginalnym algorytmem przycinania (usuwajacym "gore" ze screenshota)
    sf::Image hudImg;
    if (hudImg.loadFromFile(getTexPath("textures/hud-2.png"))) {
        int w = hudImg.getSize().x;
        int h = hudImg.getSize().y;
        
        unsigned int minX = w, minY = h;
        unsigned int maxX = 0, maxY = 0;
        for (int y = h / 2; y < h; ++y) { // skanujemy tylko dolna polowe!
            for (int x = 0; x < w; ++x) {
                if (isMagentaBackgroundAggressive(hudImg.getPixel(x, y))) {
                    hudImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
                } else if (hudImg.getPixel(x, y).a > 0) {
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

    sf::Image potkaImg;
    if (potkaImg.loadFromFile(getTexPath("textures/potka.png"))) {
        for (int y = 0; y < potkaImg.getSize().y; ++y) {
            for (int x = 0; x < potkaImg.getSize().x; ++x) {
                if (isMagentaBackgroundAggressive(potkaImg.getPixel(x, y))) potkaImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
            }
        }
        potkaTex.loadFromImage(potkaImg);
        potkaSprite.setTexture(potkaTex);
    }
    
    sf::Image iconSwordImg;
    if (iconSwordImg.loadFromFile(getTexPath("textures/ikona-miecz.png"))) {
        for (int y = 0; y < iconSwordImg.getSize().y; ++y) {
            for (int x = 0; x < iconSwordImg.getSize().x; ++x) {
                if (isMagentaBackgroundAggressive(iconSwordImg.getPixel(x, y))) iconSwordImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
            }
        }
        iconSwordTex.loadFromImage(iconSwordImg);
        iconSwordSprite.setTexture(iconSwordTex);
    }
    
    sf::Image iconCbImg;
    if (iconCbImg.loadFromFile(getTexPath("textures/ikona-kusza.png"))) {
        for (int y = 0; y < iconCbImg.getSize().y; ++y) {
            for (int x = 0; x < iconCbImg.getSize().x; ++x) {
                if (isMagentaBackgroundAggressive(iconCbImg.getPixel(x, y))) iconCbImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
            }
        }
        iconCrossbowTex.loadFromImage(iconCbImg);
        iconCrossbowSprite.setTexture(iconCrossbowTex);
    }
}

// ustawia pojedynczy piksel na buforze uwzgledniajac format rgba
void Renderer::setPixel(int x, int y, sf::Color color) {
    if (x < 0 || x >= bufferWidth || y < 0 || y >= bufferHeight) return;
    int index = (y * bufferWidth + x) * 4;
    screenPixels[index]     = color.r;
    screenPixels[index + 1] = color.g;
    screenPixels[index + 2] = color.b;
    screenPixels[index + 3] = color.a;
}

// prosty algorytm sortujacy wrogow i przedmioty od najdalszego do najblizszego
void Renderer::sortSprites(std::vector<int>& order, std::vector<double>& dist) {
    std::vector<std::pair<double, int>> sprites(order.size());
    for(size_t i = 0; i < order.size(); i++) {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }
    std::sort(sprites.begin(), sprites.end());
    // odwracamy zeby rysowac najpierw to co z tylu
    for(size_t i = 0; i < order.size(); i++) {
        dist[i] = sprites[order.size() - i - 1].first;
        order[i] = sprites[order.size() - i - 1].second;
    }
}

// glowne renderowanie
void Renderer::render(sf::RenderWindow& window, const Player& player, const Map& map) {
    // czyszczenie bufora na czarno zeby uniknac smieci
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

    // petla po kazdej pionowej kolumnie ekranu
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

        // algorytm dda szukajacy zderzenia promienia ze sciana
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

        // rysowanie sciany
        for (int y = drawStart; y < drawEnd; y++) {
            int texY = (int)texPos & (texHeight - 1);
            texPos += step;
            sf::Color color = mapTextures[texNum][texWidth * texY + texX];

            // przyciemnienie scian zeby dac iluzje cienia
            if (side == 1) {
                color.r /= 2; color.g /= 2; color.b /= 2;
            }

            setPixel(x, y, color);
        }

        // rysowanie podlogi
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
        
        // zapisanie dystansu do sciany dla danego x ekranu na potrzeby sortowania doglebnego sprite'ow
        zBuffer[x] = perpWallDist;
    }

    // tworzymy strukture pomocnicza tylko na potrzeby tej funkcji
    struct TempSprite {
        double x;
        double y;
        int texture;
        int state;
        bool mirrored;
    };
    std::vector<TempSprite> allSprites;

    // kopia wrogow do wspolnej paczki
    const auto& enemies = map.getEnemies();
    for(const auto& enemy : enemies) {
        allSprites.push_back({ enemy->getX(), enemy->getY(), enemy->getTexture(), static_cast<int>(enemy->getState()), enemy->isMirrored() });
    }

    // kopia niepodniesionych przedmiotow do tej samej wspolnej paczki
    const auto& items = map.getItems();
    for(const auto& item : items) {
        if (!item.isPickedUp) {
            allSprites.push_back({ item.x, item.y, item.texture, 0, false });
        }
    }

    // kopia lecacych pociskow
    const auto& projectiles = map.getProjectiles();
    for (const auto& proj : projectiles) {
        // pociski lataja w klatce 0
        allSprites.push_back({ proj.getX(), proj.getY(), proj.getTexture(), 0, false });
    }
    
    //  wspolne sortowanie
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


        // jedna wspolna petla rysujaca metoda lodeva

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

            // obliczamy fizyczna szerokosc sprite'a na ekranie!
            // uwaga: poniewaz zjawa i wrogowie maja klatki w dwoch rzedach (wiec ich tekstury sa wysokie na dysku),
            // musimy manualnie pomnozyc szerokosc przez 2, aby przywrocic im proporcje kwadratu (lub szerokiego plaszcza).
            int spriteWidth = abs(int(bufferHeight / (transformY))) * frame.width / frame.height;
            if (textureIndex >= 6 || textureIndex == 3) {
                // dla tekstur z dwoma rzedami klatek, mnozymy szerokosc x2, aby odzyskac wlasciwe proporcje
                spriteWidth = spriteWidth * 2;
            }

            int spriteHeight = std::abs(int(bufferHeight / transformY));
            double scaleModifier = 1.0;
            if (allSprites[currentIdx].texture == 6) { // orb
                scaleModifier = 0.3; // mniejsza skala dla orba (30% bazowego rozmiaru)
            } else if (allSprites[currentIdx].texture == 8) { // potka
                scaleModifier = 0.4; // zmniejszamy miksture leczenia (40%)
            }
            double scale = ((double)spriteHeight / frame.blockH) * scaleModifier;
            
            int drawHeight = frame.height * scale;
            int drawWidth = frame.width * scale;

            // pozycjonowanie w pionie: dol sprite'a ma dotykac podlogi pomniejszonej o bottomoffset
            int floorY = spriteHeight / 2 + bufferHeight / 2;
            int unclampedDrawEndY = floorY - frame.bottomOffset * scale;
            int drawEndY = unclampedDrawEndY;
            int drawStartY = drawEndY - drawHeight;
            
            if(drawStartY < 0) drawStartY = 0;
            if(drawEndY >= bufferHeight) drawEndY = bufferHeight - 1;

            // pozycjonowanie w poziomie: uwzgledniamy xoffset
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
                        // poprawny texy oparty na drawstarty bazowym (bez przyciecia)
                        int trueDrawStartY = unclampedDrawEndY - drawHeight;
                        int texY = int(256 * (y - trueDrawStartY) * frame.height / drawHeight) / 256;
                        
                        // odwrocenie pionowe w locie dla strzaly (tekstura 7)
                        if (allSprites[currentIdx].texture == 7) {
                            texY = frame.height - 1 - texY;
                        }

                        if (texX >= 0 && texX < frame.width && texY >= 0 && texY < frame.height) {
                            sf::Color color = frame.pixels[frame.width * texY + texX];

                            if(color.a != 0) {
                                // czerwone wypelnienie (tint) dla wrogow przy otrzymywaniu obrazen (stan 3 - hurt)
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

    // rysowanie broni
    drawWeapon(window, player);
    
    // rysowanie interfejsu (hud)
    drawHud(window, player);
}

// ==========================================
// rysowanie interfejsu i broni
// ==========================================

void Renderer::drawHud(sf::RenderWindow& window, const Player& player) {
    if (hudTexture.getSize().x == 0) return;

    sf::Vector2u texSize = hudTexture.getSize();
    
    // rysowanie krwi (krwistej kuli) - oryginalny kod uzytkownika
    float scaleY = (screenHeight * 0.25f) / texSize.y;
    float scaleX = scaleY; // utrzymujemy proporcje!
    
    float hudW = texSize.x * scaleX;
    float hudH = texSize.y * scaleY;
    float hudY = screenHeight - hudH;
    float hudX = (screenWidth - hudW) / 2.0f; // centrujemy hud na dole ekranu

    hudSprite.setScale(scaleX, scaleY);
    hudSprite.setPosition(hudX, hudY);

    // parametry idealnie wpasowujace sie w prawdziwy otwor w hud
    float radX = (texSize.x * 0.0747f) * scaleX;
    float radY = (texSize.y * 0.2868f) * scaleY;
    float orbX = hudX + (texSize.x * 0.1226f) * scaleX;
    float orbY = hudY + (texSize.y * 0.5698f) * scaleY;

    // 1. czarne tlo tylko pod kule
    sf::CircleShape bgCircle(radX);
    bgCircle.setScale(1.0f, radY / radX);
    bgCircle.setFillColor(sf::Color(10, 10, 10)); // ciemne
    bgCircle.setOrigin(radX, radX);
    bgCircle.setPosition(orbX, orbY);
    window.draw(bgCircle);

    // 2. czerwony plyn zycia
    float hpPercent = std::max(0.0f, std::min(100.0f, static_cast<float>(player.getHp()))) / 100.0f;
    sf::CircleShape fluidCircle(radX);
    fluidCircle.setScale(1.0f, radY / radX);
    fluidCircle.setFillColor(sf::Color(200, 20, 20));
    fluidCircle.setOrigin(radX, radX);
    fluidCircle.setPosition(orbX, orbY);
    window.draw(fluidCircle);
    
    // maska obcinajaca plyn od gory
    float emptyHeight = (radY * 2.0f) * (1.0f - hpPercent);
    if (emptyHeight > 0) {
        sf::RectangleShape mask(sf::Vector2f(radX * 2.5f, emptyHeight));
        mask.setFillColor(sf::Color(10, 10, 10)); // takie samo jak bgcircle
        mask.setOrigin(mask.getSize().x / 2.0f, 0);
        mask.setPosition(orbX, orbY - radY);
        window.draw(mask);
    }

    // 3. wlasciwa ramka hud na wierzch
    window.draw(hudSprite);

    // 4. efekt przezroczystego szkla
    sf::CircleShape glass(radX);
    glass.setScale(1.0f, radY / radX);
    glass.setFillColor(sf::Color(255, 255, 255, 25)); // delikatna biel
    glass.setOrigin(radX, radX);
    glass.setPosition(orbX, orbY);
    window.draw(glass);

    // --- rysowanie ikon w slotach ---
    int activeSlot = player.activeWeapon;
    
    // duze sloty (bronie) po prawej (skalibrowane do kratek na nowym hudzie)
    float slotSpacingX = 90.0f * scaleX;
    float startSlotsX = hudX + (texSize.x * 0.344f) * scaleX;
    float startSlotsY = hudY + (texSize.y * 0.405f) * scaleY;

    for(int i = 0; i < 5; i++) {
        float sx = startSlotsX + i * slotSpacingX;
        float sy = startSlotsY;
        
        // podswietlenie wybranego slotu
        if (i == activeSlot) {
            float w = slotSpacingX * 0.85f;
            float h = slotSpacingX * 0.85f;
            float radius = 8.0f;
            int points = 16;
            
            sf::ConvexShape roundedRect;
            roundedRect.setPointCount(points * 4);
            
            for(int j=0; j<points; j++) {
                float angle = (180.0f + 90.0f * j / (points - 1)) * 3.14159f / 180.0f;
                roundedRect.setPoint(j, sf::Vector2f(sx + radius + std::cos(angle)*radius, sy + radius + std::sin(angle)*radius));
            }
            for(int j=0; j<points; j++) {
                float angle = (270.0f + 90.0f * j / (points - 1)) * 3.14159f / 180.0f;
                roundedRect.setPoint(points + j, sf::Vector2f(sx + w - radius + std::cos(angle)*radius, sy + radius + std::sin(angle)*radius));
            }
            for(int j=0; j<points; j++) {
                float angle = (0.0f + 90.0f * j / (points - 1)) * 3.14159f / 180.0f;
                roundedRect.setPoint(points*2 + j, sf::Vector2f(sx + w - radius + std::cos(angle)*radius, sy + h - radius + std::sin(angle)*radius));
            }
            for(int j=0; j<points; j++) {
                float angle = (90.0f + 90.0f * j / (points - 1)) * 3.14159f / 180.0f;
                roundedRect.setPoint(points*3 + j, sf::Vector2f(sx + radius + std::cos(angle)*radius, sy + h - radius + std::sin(angle)*radius));
            }
            
            roundedRect.setFillColor(sf::Color(220, 20, 60, 100)); // polprzezroczysty karmazyn (crimson)
            roundedRect.setOutlineColor(sf::Color(220, 20, 60, 255)); // pelny karmazynowy border
            roundedRect.setOutlineThickness(3.0f);
            window.draw(roundedRect);
        }
        
        int wType = player.inventoryWeapons[i];
        if (wType == 1 && iconSwordTex.getSize().x > 0) { // miecz
            float scale = (slotSpacingX * 0.6f) / iconSwordTex.getSize().x;
            iconSwordSprite.setScale(scale, scale);
            
            float iconW = iconSwordTex.getSize().x * scale;
            float iconH = iconSwordTex.getSize().y * scale;
            iconSwordSprite.setPosition(sx + (slotSpacingX * 0.85f - iconW) / 2.0f, sy + (slotSpacingX * 0.85f - iconH) / 2.0f);
            window.draw(iconSwordSprite);
        } else if (wType == 2 && iconCrossbowTex.getSize().x > 0) { // kusza
            float scale = (slotSpacingX * 0.6f) / iconCrossbowTex.getSize().x;
            iconCrossbowSprite.setScale(scale, scale);
            
            float iconW = iconCrossbowTex.getSize().x * scale;
            float iconH = iconCrossbowTex.getSize().y * scale;
            iconCrossbowSprite.setPosition(sx + (slotSpacingX * 0.85f - iconW) / 2.0f, sy + (slotSpacingX * 0.85f - iconH) / 2.0f);
            window.draw(iconCrossbowSprite);
        }
    }
    
    // male sloty (potki) po lewej zaraz obok kuli
    float potkaX = hudX + (texSize.x * 0.231f) * scaleX;
    float potkaY = hudY + (texSize.y * 0.195f) * scaleY;
    
    if(potkaTex.getSize().x > 0) {
        float pScale = (texSize.y * 0.16f * scaleY) / potkaTex.getSize().y;
        potkaSprite.setScale(pScale, pScale);
        float potkaSpacingY = 49.0f * scaleY; // zastosowanie logicznego odstepu jak dla broni
        for(int i = 0; i < 4; i++) { // rygorystycznie maksymalnie 4 rzedy
            if(player.inventoryItems[i] == 1) { // mamy potke w tym slocie
                potkaSprite.setPosition(potkaX, potkaY + i * potkaSpacingY);
                window.draw(potkaSprite);
            }
        }
    }
    
    // licznik potek pod slotem
    int potCount = 0;
    for(int i=0; i<4; i++) if(player.inventoryItems[i] == 1) potCount++;
    
    sf::Text potkaText;
    potkaText.setFont(hudFont);
    potkaText.setString(std::to_string(potCount));
    potkaText.setCharacterSize(30);
    potkaText.setFillColor(sf::Color::White);
    potkaText.setOutlineColor(sf::Color::Black);
    potkaText.setOutlineThickness(2.0f);
    potkaText.setPosition(potkaX + 15, potkaY + 45);
    window.draw(potkaText);
}

void Renderer::drawWeapon(sf::RenderWindow& window, const Player& player) {
    int activeWep = player.getActiveWeaponType();
    
    if (activeWep == 1) { // miecz
        if (playerWeaponTexture.getSize().x == 0 || playerWeaponFrames.empty()) return;
        
        bool isSpriteSheet = (playerWeaponFrames.size() >= 3);
        double animOffsetX = 0.0;
        double animOffsetY = 0.0;
        double rotation = 0.0;
        int frameIndex = 0; 
        
        if (player.getAttackTimer() > 0.0) {
            double progress = player.getAttackTimer() / 0.5; // od 1.0 w dol
            if (isSpriteSheet) {
                if (progress > 0.75) frameIndex = 1;
                else if (progress > 0.5) frameIndex = 2;
                else if (progress > 0.2) frameIndex = 3;
                else frameIndex = 0;
            } else {
                if (progress > 0.7) {
                    double p = (1.0 - progress) / 0.3;
                    animOffsetX = -150.0 * p;
                    animOffsetY = 50.0 * p;
                    rotation = 20.0 * p;
                } else if (progress > 0.3) {
                    double p = (0.7 - progress) / 0.4;
                    animOffsetX = -150.0 + 600.0 * p;
                    animOffsetY = 50.0 - 150.0 * p; 
                    rotation = 20.0 - 100.0 * p;
                } else {
                    double p = progress / 0.3;
                    animOffsetX = 450.0 * p;
                    animOffsetY = -100.0 * p;
                    rotation = -80.0 * p;
                }
            }
        } else {
            animOffsetX = std::sin(player.getX() * 5.0) * 20.0f;
            animOffsetY = std::abs(std::cos(player.getY() * 5.0)) * 20.0f;
        }
        
        if (frameIndex >= playerWeaponFrames.size()) frameIndex = playerWeaponFrames.size() - 1;
        
        sf::IntRect currentFrame = playerWeaponFrames[frameIndex];
        playerWeaponSprite.setTextureRect(currentFrame);
        playerWeaponSprite.setOrigin(currentFrame.width / 2.0f, currentFrame.height);
        
        // odbijamy lustrzanie tylko pierwsza klatke
        if (frameIndex == 0 && isSpriteSheet) {
            playerWeaponSprite.setScale(-1.0f, 1.0f); 
        } else {
            playerWeaponSprite.setScale(1.0f, 1.0f);
        }
        
        // miecz dosuniety idealnie do prawej krawedzi ekranu
        float swordX = screenWidth - (currentFrame.width * 0.8f) / 2.0f;
        playerWeaponSprite.setPosition(swordX - animOffsetX, screenHeight - 20.0f + animOffsetY);
        playerWeaponSprite.setRotation(rotation);
        window.draw(playerWeaponSprite);
        
    } else if (activeWep == 2) { // kusza
        if (crossbowTexture.getSize().x == 0) return;
        
        bool isSpriteSheet = true;
        
        float scale = 1.0f; 
        
        double animOffsetX = 0.0;
        double animOffsetY = 0.0;
        double rotation = 0.0;
        int frameIndex = 0;
        
        if (player.getAttackTimer() > 0.0) {
            double progress = player.getAttackTimer() / 0.5; 
            
            if (progress > 0.75) frameIndex = 1;
            else if (progress > 0.5) frameIndex = 2;
            else if (progress > 0.2) frameIndex = 3;
            else frameIndex = 0;
            
            if (progress > 0.8) {
                double p = (1.0 - progress) / 0.2;
                animOffsetY = p * 100.0f;
            } else {
                double p = progress / 0.8;
                animOffsetY = p * 100.0f; 
            }
        } else {
            animOffsetX = std::sin(player.getX() * 5.0) * 15.0f;
            animOffsetY = std::abs(std::cos(player.getY() * 5.0)) * 15.0f;
        }
        
        // podzial matematyczny na siatke 2x2 (zgodnie z ulozeniem obrazka kusza.png)
        int frameW = crossbowTexture.getSize().x / 2;
        int frameH = crossbowTexture.getSize().y / 2;
        
        // wyliczanie kolumny i rzedu (klatki 0,1,2,3)
        int col = frameIndex % 2;
        int row = frameIndex / 2;
        
        sf::IntRect currentRect = sf::IntRect(col * frameW, row * frameH, frameW, frameH);
        
        crossbowSprite.setTextureRect(currentRect);
        
        sf::FloatRect bounds = crossbowSprite.getLocalBounds();
        if (bounds.width > 0 && bounds.height > 0) {
            crossbowSprite.setOrigin(currentRect.width / 2.0f, currentRect.height);
            
            float scale = 1.1f; // kusza lekko 'do przodu' (zblizona do kamery)
            crossbowSprite.setScale(scale, scale);
            
            float crossbowX = screenWidth / 2.0f; // kusza na samym srodku
            // kusza opuszczona, by srodek celownika zgrywal sie z rzeczywistym miejscem wypuszczenia strzaly
            crossbowSprite.setPosition(crossbowX - animOffsetX, screenHeight + animOffsetY + 25.0f);
            crossbowSprite.setRotation(rotation);
            window.draw(crossbowSprite);
        }
    }
}


