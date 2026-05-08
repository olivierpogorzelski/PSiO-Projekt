#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

using namespace std;
//Dane podstawowe dotyczącze rozmiaru mapy
constexpr int mapWidth = 24;
constexpr int mapHeight = 24;
//Dane dotyczące wielkości okienka
constexpr int screenWidth = 640;
constexpr int screenHeight = 480;

// Podstawowa mapa tymczasowego świata
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
    // Ustawienie renderu okienka
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Test Raycastingu");
    window.setFramerateLimit(60);

    //pozycja gracza
    double posX=22, posY=12;
    //wersor/kierunek kamery
    double dirX=-1, dirY=0;
    //pozycja kamery FOV
    double planeX=0, planeY=0.66;

    sf::Clock clock; // zegar, za pomocą którego liczymy odstępy między klatkami

    while(window.isOpen()){
        sf::Event event;
        while(window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);

        // tablica z rysowanymi liniami, jest 2 razy większa bo potrzebne są dwa wierzchołki
        sf::VertexArray lines(sf::Lines, screenWidth * 2);

        // pętla w której liczymy każdą linię dla poszczególnego x z podglądu kamery
        for (int x = 0; x < screenWidth; x++){
            // pozycja x dla kamery
            double cameraX = 2*x/double(screenWidth) - 1;
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
            int hit=0;
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
            int lineHeight = (int)(screenHeight / perpWallDist);

            // znajdujemy najniższy i najwyższy piksel, który chcemy wypełnić linią.
            int drawStart = -lineHeight / 2 + screenHeight / 2;
            if(drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + screenHeight / 2;
            if(drawEnd >= screenHeight) drawEnd = screenHeight - 1;

            // wybór koloru
            sf::Color color;
            switch(worldMap[mapX][mapY])
            {
            case 1:  color = sf::Color::Red;  break;
            case 2:  color = sf::Color::Green;  break;
            case 3:  color = sf::Color::Blue;   break;
            case 4:  color = sf::Color::White;  break;
            default: color = sf::Color::Yellow; break;
            }

            // różne cieniowania dla x i y
            if (side == 1) {
                color.r /= 2; color.g /= 2; color.b /= 2;
            }

            // tworzymy wektory
            lines[x * 2].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(drawStart));
            lines[x * 2].color = color;
            lines[x * 2 + 1].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(drawEnd));
            lines[x * 2 + 1].color = color;
        }

        window.draw(lines);

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