#include "AssetManager.h"
#include <iostream>

void AssetManager::loadAllTextures() {
    playerTexture.loadFromFile(PROJECT_DIR "assets/textures/Soldier-Walk.png");
    attackRightTex.loadFromFile(PROJECT_DIR "assets/textures/atakprawo.png");
    attackLeftTex.loadFromFile(PROJECT_DIR "assets/textures/ataklewo.png");
    enemyTexD.loadFromFile(PROJECT_DIR "assets/textures/BD_Walk.png");
    enemyTexS.loadFromFile(PROJECT_DIR "assets/textures/BS_Walk.png");
    enemyTexU.loadFromFile(PROJECT_DIR "assets/textures/BU_Walk.png");
    wolfTexD.loadFromFile(PROJECT_DIR "assets/textures/WD_Walk.png");
    wolfTexS.loadFromFile(PROJECT_DIR "assets/textures/WS_Walk.png");
    wolfTexU.loadFromFile(PROJECT_DIR "assets/textures/WU_Walk.png");
    ogreTexD.loadFromFile(PROJECT_DIR "assets/textures/D_Walk.png");
    ogreTexS.loadFromFile(PROJECT_DIR "assets/textures/S_Walk.png");
    ogreTexU.loadFromFile(PROJECT_DIR "assets/textures/U_Walk.png");

    towerIdleTexs.resize(7);
    towerUpgradeTexs.resize(7);
    for (int i = 0; i < 7; ++i) {
        towerIdleTexs[i].loadFromFile(PROJECT_DIR "assets/textures/" + std::to_string(i + 1) + "idle.png");
        towerUpgradeTexs[i].loadFromFile(PROJECT_DIR "assets/textures/" + std::to_string(i + 1) + "upgrade.png");
    }
}

void AssetManager::loadAudio() {
    gunshotBuf.loadFromFile(PROJECT_DIR "assets/sound/gunshot.mp3");
    sniperBuf.loadFromFile(PROJECT_DIR "assets/sound/sniper_gunshot.mp3");
    buildBuf.loadFromFile(PROJECT_DIR "assets/sound/build.mp3");
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
