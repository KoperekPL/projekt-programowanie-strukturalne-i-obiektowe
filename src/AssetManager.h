#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class AssetManager {
public:
    sf::Texture playerTexture;
    sf::Texture enemyTexD;
    sf::Texture enemyTexS;
    sf::Texture enemyTexU;
    sf::Texture wolfTexD;
    sf::Texture wolfTexS;
    sf::Texture wolfTexU;
    sf::Texture ogreTexD;
    sf::Texture ogreTexS;
    sf::Texture ogreTexU;

    std::vector<sf::Texture> towerIdleTexs;
    std::vector<sf::Texture> towerUpgradeTexs;

    void loadAllTextures();
};

#endif // ASSETMANAGER_H
