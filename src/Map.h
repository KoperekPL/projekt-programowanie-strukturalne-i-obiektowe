#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Map {
private:
    std::vector<sf::Vector2f> pathPoints;
    std::vector<sf::Vector2f> towerSpots;
    std::vector<sf::Vector2f> bridgeSpots;
    std::vector<sf::Vector2f> storeSpots;
    float tileSize;

public:
    Map(float tSize = 20.f);

    bool loadFromFile(const std::string& filename);
    
    void draw(sf::RenderWindow& window) const;

    const std::vector<sf::Vector2f>& getPathPoints() const { return pathPoints; }
    const std::vector<sf::Vector2f>& getTowerSpots() const { return towerSpots; }
    const std::vector<sf::Vector2f>& getStoreSpots() const { return storeSpots; }
};

#endif
