#include "AssetManager.h"
#include <iostream>

void AssetManager::loadAllTextures() {
    playerTexture.loadFromFile("../../../assets/textures/Soldier-Walk.png");
    enemyTexD.loadFromFile("../../../assets/textures/D_Walk.png");
    enemyTexS.loadFromFile("../../../assets/textures/S_Walk.png");
    enemyTexU.loadFromFile("../../../assets/textures/U_Walk.png");

    towerIdleTexs.resize(7);
    towerUpgradeTexs.resize(7);
    for (int i = 0; i < 7; ++i) {
        towerIdleTexs[i].loadFromFile("../../../assets/textures/" + std::to_string(i + 1) + "idle.png");
        towerUpgradeTexs[i].loadFromFile("../../../assets/textures/" + std::to_string(i + 1) + "upgrade.png");
    }
}
