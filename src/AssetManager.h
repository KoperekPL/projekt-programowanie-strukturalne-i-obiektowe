#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <memory>

class AssetManager {
public:
    sf::Texture playerTexture;
    sf::Texture attackRightTex;
    sf::Texture attackLeftTex;
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

    sf::SoundBuffer gunshotBuf;
    sf::SoundBuffer sniperBuf;
    sf::SoundBuffer buildBuf;
    std::vector<std::unique_ptr<sf::Sound>> activeSounds;

    void loadAllTextures();
    void loadAudio();
    void playSound(bool isSniper, float volume);
    void updateSounds();
};

#endif // ASSETMANAGER_H
