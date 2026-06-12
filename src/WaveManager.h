#ifndef WAVEMANAGER_H
#define WAVEMANAGER_H

#include <vector>
#include <string>
#include "Enemy.h"
#include <map>
#include <memory>
#include "GameObject.h"
#include "AssetManager.h"
#include "Map.h"

class WaveManager {
public:
    int currentWave = 1;
    int maxWaves = 10;
    int enemiesToSpawn = 0;
    int enemiesSpawned = 0;
    float spawnTimer = 0.f;
    float spawnInterval = 2.0f;
    bool waveActive = false;
    float timeBetweenWaves = 10.f;
    float waveCooldown = 10.f;

    void startWave(int wave);
    void update(float dt, std::vector<std::shared_ptr<GameObject>>& objects, const std::map<std::string, EnemyStats>& configs, const Map& map, AssetManager& assets);
};

#endif