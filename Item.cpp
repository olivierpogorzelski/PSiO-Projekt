#include "Item.hpp"

Item::Item(float startX, float startY, int textureI): x(startX), y(startY), isPickedUp(false), texture(textureI) {}
bool Item::checkCollision(double playerX, double playerY)
{
    if (isPickedUp) return false;

    const float pickupRadius = 0.5f;
    float dx = playerX - x;
    float dy = playerY - y;

    if ((dx * dx + dy * dy) < (pickupRadius * pickupRadius)) {
        isPickedUp = true; // przedmiot zostaje oznaczony jako zebrany
        return true;       // informujemy mape, ze trzeba nalozyc efekt na gracza

    }
    return false;
}


