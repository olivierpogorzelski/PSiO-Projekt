#pragma once

class Item
{
public:
    Item(float startX, float startY, int textureI);
    float x;
    float y;
    bool isPickedUp;     // flaga informująca, czy przedmiot już zebrano (jeśli tak, nie renderujemy go i ignorujemy kolizje)
    int texture;
    bool checkCollision(double playerX, double playerY);
};


