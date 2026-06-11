#include "Item.hpp"
#include "Player.hpp"
#include "SoundManager.hpp"
#include <cmath>

Item::Item(float startX, float startY, int textureI) : GameObject(startX, startY), texture(textureI) {}

bool Item::update(double frameTime, Player& player, Map& map) {
    // sprawdzanie kolizji
    double dx = x - player.getX();
    double dy = y - player.getY();
    double distance = std::sqrt(dx * dx + dy * dy);
    
    // zbieranie przedmiotu, jeśli gracz jest wystarczająco blisko
    if (distance < 0.5) {
        bool pickedUp = false;
        
        if (texture == 10) { // Klucz
            for(int i = 0; i < 5; i++) {
                if (player.inventoryWeapons[i] == 0) {
                    player.inventoryWeapons[i] = 3; // 3 oznacza klucz
                    pickedUp = true;
                    break;
                }
            }
        } else { // potka (texture == 8)
            for(int i = 0; i < 4; i++) {
                if(player.inventoryItems[i] == 0) {
                    player.inventoryItems[i] = 1;
                    pickedUp = true;
                    break;
                }
            }
        }
        
        if (pickedUp) {
            SoundManager::getInstance().playSound("gracz_podniesienie");
            return true; // obiekt powinien zostać usunięty z mapy
        }
    }
    
    return false; // nie został podniesiony, nie usuwaj
}
