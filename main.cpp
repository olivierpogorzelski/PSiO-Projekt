#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

// rozmiar mapy
constexpr int mapWidth = 24;
constexpr int mapHeight = 24;

// wielkość okna
constexpr int screenWidth = 1920;
constexpr int screenHeight = 1080;

// rozdzielczość wewnętrznego bufora (4 razy mniejsza dla wzrostu FPS)
constexpr int bufferWidth = 480;
constexpr int bufferHeight = 270;

// rozmiary tekstur
constexpr int texWidth = 64;
constexpr int texHeight = 64;

// podstawowa mapa tymczasowego świata
int worldMap[mapWidth][mapHeight] =
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,2,2,2,2,2,0,0,3,3,3,3,3,3,0,0,4,4,4,0,0,1},
        {1,0,2,0,0,0,0,2,0,0,3,0,0,0,0,3,0,0,4,0,4,0,0,1},
        {1,0,2,0,5,5,0,2,0,0,3,0,0,0,0,3,0,0,4,0,4,0,0,1},
        {1,0,2,0,5,5,0,2,0,0,3,3,0,3,3,3,0,0,4,4,4,0,0,1},
        {1,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,2,2,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,5,0,5,0,5,0,5,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,3,0,3,0,0,2,2,2,2,2,2,2,2,2,2,2,0,0,5,0,0,1},
        {1,0,3,0,3,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,3,0,3,0,0,2,0,0,4,0,0,0,0,4,0,2,0,0,5,0,0,1},
        {1,0,3,3,3,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,2,0,0,4,0,0,0,0,4,0,2,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,2,2,2,2,2,0,2,2,2,2,2,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int main()
{
    // ustawienie renderu okienka
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Test Raycastingu z Teksturami Naprawiony");
    window.setFramerateLimit(60);

    // pozycja gracza
    double posX = 22, posY = 12;
    // wersor/kierunek kamery
    double dirX = -1, dirY = 0;
    // pozycja kamery FOV
    double planeX = 0, planeY = 0.66;

    // generowanie tekstur
    std::vector<std::vector<sf::Color>> textures(8, std::vector<sf::Color>(texWidth * texHeight));
    for (int x = 0; x < texWidth; x++) {
        for (int y = 0; y < texHeight; y++) {
            int xorcolor = (x * 256 / texWidth) ^ (y * 256 / texHeight);
            int xycolor = y * 128 / texHeight + x * 128 / texWidth;
            textures[0][texWidth * y + x] = sf::Color(254, xorcolor, xorcolor); // Czerwona
            textures[1][texWidth * y + x] = sf::Color(xycolor, 254, xycolor);   // Zielona
            textures[2][texWidth * y + x] = sf::Color(xorcolor, xorcolor, 254); // Niebieska
            textures[3][texWidth * y + x] = sf::Color(xorcolor, xorcolor, xorcolor); // Szara
            textures[4][texWidth * y + x] = sf::Color(254, 254, xorcolor);      // Żółta
            textures[5][texWidth * y + x] = sf::Color(80, 80, 80);              // Podłoga
            textures[6][texWidth * y + x] = sf::Color(40, 40, 40);              // Sufit
        }
    }

    // tablica z buforem pikseli, teraz w znacznie mniejszej rozdzielczości
    std::vector<sf::Uint8> screenPixels(bufferWidth * bufferHeight * 4, 0);
    sf::Texture screenTexture;
    screenTexture.create(bufferWidth, bufferHeight);

    // wyłączamy wygładzanie, aby piksele po powiększeniu były ostre (można wyłączyć, ale to jest dla klimatu gier lat 90)
    screenTexture.setSmooth(false);

    sf::Sprite screenSprite(screenTexture);
    // Skalujemy mały bufor tak, aby wypełnił całe duże okno Full HD
    screenSprite.setScale((float)screenWidth / bufferWidth, (float)screenHeight / bufferHeight);

    // funkcja pomocnicza do zapisu pikseli do zoptymalizowanego bufora
    auto setPixel = [&](int x, int y, sf::Color color) {
        if (x < 0 || x >= bufferWidth || y < 0 || y >= bufferHeight) return;
        int index = (y * bufferWidth + x) * 4;
        screenPixels[index]     = color.r;
        screenPixels[index + 1] = color.g;
        screenPixels[index + 2] = color.b;
        screenPixels[index + 3] = 255;
    };

    sf::Clock clock; // zegar, za pomocą którego liczymy odstępy między klatkami

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        // pętla w której liczymy każdą linię dla poszczególnego x z podglądu kamery
        for (int x = 0; x < bufferWidth; x++){
            // pozycja x dla kamery
            double cameraX = 2 * x / double(bufferWidth) - 1;
            // kierunki promienia
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            // pozycja obecnego kwadratu, na którym się znajdujemy
            int mapX = int(posX);
            int mapY = int(posY);

            // odległości do najbliższych krawędzi w zależności od wymiaru, dla kwadratu na którym się znajdujemy
            double sideDistX;
            double sideDistY;
            // odległości z punktów styków dla danego wymiaru następnych najbliższych kwadratów w zasięgu promienia z punktami styku dla kolejnego kwadratu
            double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1/rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1/rayDirY);
            // prostopadła odległość z pozycji kamery do danej ściany
            double perpWallDist;

            // kierunki do których zmierza promień
            int stepX; // -1 lub 1 dla lewa/prawa
            int stepY; // -1 lub 1 dla przodu/tyłu

            // czy promień trafił ścianę? jeśli tak, stan zmieni się na 1
            int hit = 0;
            // czy to była ściana z przodu/tyłu czy z lewa/prawa?
            int side;

            // obliczanie odległości
            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

            // dopóki nie trafimy!
            while (hit == 0)
            {
                // przejdzie do następnego kwadratu, dla wymiaru x lub y!
                if (sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                // czy w końcu trafiło?
                if (worldMap[mapX][mapY] > 0) hit = 1;
            }

            // liczony jest dystans rzutu na kierunek kamery
            if(side == 0) perpWallDist = (sideDistX - deltaDistX);
            else          perpWallDist = (sideDistY - deltaDistY);

            // długość rysowanej linii
            int lineHeight = (int)(bufferHeight / perpWallDist);

            // znajdujemy najniższy i najwyższy piksel, który chcemy wypełnić linią.
            int drawStart = -lineHeight / 2 + bufferHeight / 2;
            if(drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + bufferHeight / 2;
            if(drawEnd >= bufferHeight) drawEnd = bufferHeight - 1;

            int texNum = worldMap[mapX][mapY] - 1;

            double wallX;
            if (side == 0) wallX = posY + perpWallDist * rayDirY;
            else           wallX = posX + perpWallDist * rayDirX;
            wallX -= floor((wallX));

            int texX = int(wallX * double(texWidth));
            if(side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
            if(side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

            double step = 1.0 * texHeight / lineHeight;
            double texPos = (drawStart - bufferHeight / 2 + lineHeight / 2) * step;

            // obliczanie pozycji podłogi/sufitu w świecie PRZED uruchomieniem pętli renderujących piksele
            double floorXWall, floorYWall;
            if(side == 0 && rayDirX > 0) { floorXWall = mapX; floorYWall = mapY + wallX; }
            else if(side == 0 && rayDirX < 0) { floorXWall = mapX + 1.0; floorYWall = mapY + wallX; }
            else if(side == 1 && rayDirY > 0) { floorXWall = mapX + wallX; floorYWall = mapY; }
            else { floorXWall = mapX + wallX; floorYWall = mapY + 1.0; }

            // od góry ekranu do początku ściany
            for (int y = 0; y < drawStart; y++) {
                int floorY = bufferHeight - y - 1; // Mapowanie symetryczne dla wysokości
                if (2 * floorY - bufferHeight == 0) continue; // Zabezpieczenie przed horyzontem (dzielenie przez 0)

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

            // od początku ściany do jej końca
            for (int y = drawStart; y < drawEnd; y++) {
                int texY = (int)texPos & (texHeight - 1);
                texPos += step;
                sf::Color color = textures[texNum][texWidth * texY + texX];

                // różne cieniowania dla x i y
                if (side == 1) {
                    color.r /= 2; color.g /= 2; color.b /= 2;
                }

                setPixel(x, y, color);
            }

            // od końca ściany do samego dołu ekranu
            for(int y = drawEnd; y < bufferHeight; y++) {
                if (2 * y - bufferHeight == 0) continue; // Zabezpieczenie przed horyzontem (dzielenie przez 0)

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
        }

        // aktualizacja zawartości tekstury bufora
        screenTexture.update(screenPixels.data());
        // rysujemy powiększonego sprite'a na pełnym ekranie
        window.draw(screenSprite);

        // różnica czasu między klatkami
        double frameTime = clock.restart().asSeconds();
        double moveSpeed = frameTime * 5.0;
        double rotSpeed = frameTime * 3.0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            // idziemy do przodu, jeśli nie ma przed nami ściany
            if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            // idziemy do tyłu, jeśli nie ma za nami ściany
            if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            // obracamy w prawo, jak i kierunek kamery, jak i jej płaszczyznę
            double oldDirX = dirX;
            dirX = dirX * std::cos(-rotSpeed) - dirY * std::sin(-rotSpeed);
            dirY = oldDirX * std::sin(-rotSpeed) + dirY * std::cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * std::cos(-rotSpeed) - planeY * std::sin(-rotSpeed);
            planeY = oldPlaneX * std::sin(-rotSpeed) + planeY * std::cos(-rotSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            // obracamy w lewo, jak i kierunek kamery, jak i jej płaszczyznę
            double oldDirX = dirX;
            dirX = dirX * std::cos(rotSpeed) - dirY * std::sin(rotSpeed);
            dirY = oldDirX * std::sin(rotSpeed) + dirY * std::cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * std::cos(rotSpeed) - planeY * std::sin(rotSpeed);
            planeY = oldPlaneX * std::sin(rotSpeed) + planeY * std::cos(rotSpeed);
        }

        window.display();
    }
    return 0;
}