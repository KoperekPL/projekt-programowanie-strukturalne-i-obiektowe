#include "AssetManager.h"
#include <iostream>

void AssetManager::loadAllTextures() {
    playerTexture.loadFromFile("../../../assets/textures/Soldier-Walk.png");
    enemyTexD.loadFromFile("../../../assets/textures/BD_Walk.png");
    enemyTexS.loadFromFile("../../../assets/textures/BS_Walk.png");
    enemyTexU.loadFromFile("../../../assets/textures/BU_Walk.png");
    wolfTexD.loadFromFile("../../../assets/textures/WD_Walk.png");
    wolfTexS.loadFromFile("../../../assets/textures/WS_Walk.png");
    wolfTexU.loadFromFile("../../../assets/textures/WU_Walk.png");
    ogreTexD.loadFromFile("../../../assets/textures/D_Walk.png");
    ogreTexS.loadFromFile("../../../assets/textures/S_Walk.png");
    ogreTexU.loadFromFile("../../../assets/textures/U_Walk.png");

    towerIdleTexs.resize(7);
    towerUpgradeTexs.resize(7);
    for (int i = 0; i < 7; ++i) {
        towerIdleTexs[i].loadFromFile("../../../assets/textures/" + std::to_string(i + 1) + "idle.png");
        towerUpgradeTexs[i].loadFromFile("../../../assets/textures/" + std::to_string(i + 1) + "upgrade.png");
    }
}

void AssetManager::loadAudio() {
    gunshotBuf.loadFromFile("../../../assets/sound/gunshot.mp3");
    sniperBuf.loadFromFile("../../../assets/sound/sniper_gunshot.mp3");
    buildBuf.loadFromFile("../../../assets/sound/build.mp3");
}

void AssetManager::playSound(bool isSniper, float volume) {
    if (activeSounds.size() > 20) return;
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(isSniper ? sniperBuf : gunshotBuf);
    sound->setVolume(volume);
    sound->play();
    activeSounds.push_back(std::move(sound));
}

void AssetManager::updateSounds() {
    activeSounds.erase(
        std::remove_if(activeSounds.begin(), activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& s) { return s->getStatus() == sf::Sound::Stopped; }
        ),
        activeSounds.end()
    );
}
