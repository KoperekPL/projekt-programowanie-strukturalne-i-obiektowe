#ifndef MAP_H
#define MAP_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <cstdlib>

class Map {
private:
    std::vector<sf::Vector2f> pathPointsA;
    std::vector<sf::Vector2f> pathPointsB;
    std::vector<sf::Vector2f> towerSpots;
    std::vector<sf::Vector2f> bridgeSpots;
    std::vector<sf::Vector2f> storeSpots;
    std::vector<sf::Vector2f> healerSpots;
    std::vector<std::string> grid;

    // terrain tile textures
    sf::Texture texGrass;      // trawa.png – tile trawy
    sf::Texture texPath;       // sciezka.png – tile ścieżki

    // decoration textures (losowe na trawie)
    static const int NUM_DECO = 10;
    sf::Texture texDeco[NUM_DECO]; // drzewo, krzak1, krzak2, pien, trawa1-6

    struct DecoInstance {
        int texIndex;           // indeks w texDeco[]
        sf::Vector2f position;
        float scale;
    };
    std::vector<DecoInstance> decorations;

    bool hasBg;
    float tileSize;

public:
    Map(float tSize = 20.f);

    bool loadFromFile(const std::string& filename);

    void draw(sf::RenderWindow& window) const;

    // Stara nazwa - zachowana dla kompatybilności, zwraca ścieżkę A
    const std::vector<sf::Vector2f>& getPathPoints() const { return pathPointsA; }

    const std::vector<sf::Vector2f>& getPathA() const { return pathPointsA; }
    const std::vector<sf::Vector2f>& getPathB() const { return pathPointsB; }

    // Losuje jedną z dwóch ścieżek (jeśli mapa nie ma rozwidlenia, A == B)
    const std::vector<sf::Vector2f>& getRandomPath() const {
        return (rand() % 2 == 0) ? pathPointsA : pathPointsB;
    }

    const std::vector<sf::Vector2f>& getTowerSpots() const { return towerSpots; }
    const std::vector<sf::Vector2f>& getBridgeSpots() const { return bridgeSpots; }
    const std::vector<sf::Vector2f>& getStoreSpots() const { return storeSpots; }
    const std::vector<sf::Vector2f>& getHealerSpots() const { return healerSpots; }
    float getTileSize() const { return tileSize; }
};

#endif
