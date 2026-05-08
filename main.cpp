#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

using namespace std;

constexpr int mapWidth = 24;
constexpr int mapHeight = 24;
constexpr int screenWidth = 640;
constexpr int screenHeight = 480;

int worldMap[mapWidth][mapHeight]=
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
        {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
        {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

int main()
{
    // Powrót do starego VideoMode
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Raycasting Test (SFML 2.5)");
    window.setFramerateLimit(60);

    double posX=22, posY=12;
    double dirX=-1, dirY=0;
    double planeX=0, planeY=0.66;

    sf::Clock clock;

    while(window.isOpen()){
        // Klasyczna pętla zdarzeń
        sf::Event event;
        while(window.pollEvent(event)){
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // WYCZYSZCZENIE EKRANU - to jest ta linijka, której brakowało wcześniej
        window.clear(sf::Color::Black);

        // Zmiana na sf::Lines
        sf::VertexArray lines(sf::Lines, screenWidth * 2);

        for (int x = 0; x < screenWidth; x++){
            double cameraX = 2*x/double(screenWidth) - 1;
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

            int hit=0;
            int side;

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

            while (hit == 0)
            {
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
                if (worldMap[mapX][mapY] > 0) hit = 1;
            }

            if(side == 0) perpWallDist = (sideDistX - deltaDistX);
            else          perpWallDist = (sideDistY - deltaDistY);

            int lineHeight = (int)(screenHeight / perpWallDist);
            int drawStart = -lineHeight / 2 + screenHeight / 2;
            if(drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + screenHeight / 2;
            if(drawEnd >= screenHeight) drawEnd = screenHeight - 1;

            sf::Color color;
            switch(worldMap[mapX][mapY])
            {
            case 1:  color = sf::Color::Red;  break;
            case 2:  color = sf::Color::Green;  break;
            case 3:  color = sf::Color::Blue;   break;
            case 4:  color = sf::Color::White;  break;
            default: color = sf::Color::Yellow; break;
            }

            if (side == 1) {
                color.r /= 2; color.g /= 2; color.b /= 2;
            }

            // Powrót do klasycznego tworzenia wektora
            lines[x * 2].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(drawStart));
            lines[x * 2].color = color;
            lines[x * 2 + 1].position = sf::Vector2f(static_cast<float>(x), static_cast<float>(drawEnd));
            lines[x * 2 + 1].color = color;
        }

        window.draw(lines);

        double frameTime = clock.restart().asSeconds();
        double moveSpeed = frameTime * 5.0;
        double rotSpeed = frameTime * 3.0;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        {
            if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            double oldDirX = dirX;
            dirX = dirX * std::cos(-rotSpeed) - dirY * std::sin(-rotSpeed);
            dirY = oldDirX * std::sin(-rotSpeed) + dirY * std::cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * std::cos(-rotSpeed) - planeY * std::sin(-rotSpeed);
            planeY = oldPlaneX * std::sin(-rotSpeed) + planeY * std::cos(-rotSpeed);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
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