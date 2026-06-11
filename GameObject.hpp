#pragma once

class Player;
class Map;

// Glowna klasa bazowa dla wszystkich obiektow (wrogow, itemow, pociskow)
class GameObject {
public:
    GameObject(double startX, double startY) : x(startX), y(startY) {}
    virtual ~GameObject() = default;
    
    // update zwraca true jesli obiekt ma zostac usuniety z gry
    virtual bool update(double frameTime, Player& player, Map& map) { return false; }
    
    virtual int getTexture() const = 0;
    virtual int getState() const { return 0; }
    virtual bool isMirrored() const { return false; }
    
    double getX() const { return x; }
    double getY() const { return y; }
    void setX(double newX) { x = newX; }
    void setY(double newY) { y = newY; }

protected:
    double x;
    double y;
};
