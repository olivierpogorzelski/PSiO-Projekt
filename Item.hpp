#pragma once
#include "GameObject.hpp"

class Item : public GameObject
{
public:
    Item(float startX, float startY, int textureI);
    int texture;
    
    // zwraca true, jeśli gracz podniesie item (wtedy usuniemy go z tablicy entities)
    bool update(double frameTime, Player& player, Map& map) override;
    
    int getTexture() const override { return texture; }
};




