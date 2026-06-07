#pragma once

class Item
{
public:
    Item(float startX, float startY, int textureI);
    float x;
    float y;
    bool isPickedUp;     // flaga informujaca, czy przedmiot juz zebrano (jesli tak, nie renderujemy go i ignorujemy kolizje)
    int texture;
    bool checkCollision(double playerX, double playerY);
};




