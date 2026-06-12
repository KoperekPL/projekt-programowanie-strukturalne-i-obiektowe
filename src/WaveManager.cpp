#include "WaveManager.h"
#include <iostream>

void WaveManager::startWave(int wave) {
    currentWave = wave;
    enemiesToSpawn = 8 + wave * 5; // Więcej mobów, fale trwają dłużej
    enemiesSpawned = 0;
    spawnInterval = 2.0f - (wave * 0.1f);
    if (spawnInterval < 0.5f) spawnInterval = 0.5f;
    spawnTimer = 0.f;
    waveActive = true;
}

void WaveManager::update(float dt, std::vector<std::shared_ptr<GameObject>>& objects, const std::map<std::string, EnemyStats>& configs, const Map& map, AssetManager& assets) {
    if (!waveActive) {
        waveCooldown -= dt;
        if (waveCooldown <= 0.f && currentWave <= maxWaves) {
            startWave(currentWave);
        }
        return;
    }

    if (enemiesSpawned < enemiesToSpawn) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.f) {
            float randomVariance = ((rand() % 100) / 100.0f) * spawnInterval * 1.5f;
            spawnTimer = spawnInterval + randomVariance;

            if (!configs.empty() && !map.getPathA().empty()) {
                std::string chosenEnemy = "Enemy_Basic"; // Default for wave 1 and 2

                if (currentWave > 2) {
                    int r = rand() % 100;
                    // The chance for special enemies increases with the wave number
                    int specialChance = (currentWave - 2) * 15;
                    if (specialChance > 90) specialChance = 90; // Max 90% chance for special enemies

                    if (r < specialChance) {
                        if (currentWave >= 5) {
                            // Can spawn Wolf or Ogre
                            if (rand() % 2 == 0 && configs.find("Enemy_Ogre") != configs.end()) {
                                chosenEnemy = "Enemy_Ogre";
                            } else if (configs.find("Enemy_Wolf") != configs.end()) {
                                chosenEnemy = "Enemy_Wolf";
                            }
                        } else {
                            // Only Wolf on waves 3 and 4
                            if (configs.find("Enemy_Wolf") != configs.end()) {
                                chosenEnemy = "Enemy_Wolf";
                            }
                        }
                    }
                }

                // Make sure the chosen enemy exists in config, otherwise fallback
                if (configs.find(chosenEnemy) == configs.end()) {
                    chosenEnemy = configs.begin()->first;
                }

                EnemyStats stats = configs.at(chosenEnemy);

                // Scale HP significantly for later waves
                if (currentWave >= 2) {
                    float hpMultiplier = 1.0f + (currentWave - 1) * 0.6f;
                    stats.maxHp = static_cast<int>(stats.maxHp * hpMultiplier);
                }

                const std::string& ename = chosenEnemy;
                const sf::Texture* tD = &assets.enemyTexD;
                const sf::Texture* tS = &assets.enemyTexS;
                const sf::Texture* tU = &assets.enemyTexU;
                if (ename == "Enemy_Wolf") {
                    tD = &assets.wolfTexD;
                    tS = &assets.wolfTexS;
                    tU = &assets.wolfTexU;
                } else if (ename == "Enemy_Ogre") {
                    tD = &assets.ogreTexD;
                    tS = &assets.ogreTexS;
                    tU = &assets.ogreTexU;
                }

                // Losujemy ścieżkę (A lub B) dla tego konkretnego wroga
                const auto& chosenPath = map.getRandomPath();

                auto enemy = std::make_shared<Enemy>(chosenPath[0], stats, tD, tS, tU, &chosenPath);
                objects.push_back(enemy);
                enemiesSpawned++;
            }
        }
    } else {
        bool enemiesAlive = false;
        for (const auto& obj : objects) {
            if (dynamic_cast<Enemy*>(obj.get()) != nullptr) {
                enemiesAlive = true;
                break;
            }
        }
        if (!enemiesAlive) {
            waveActive = false;
            currentWave++;
            waveCooldown = timeBetweenWaves;
        }
    }
}
