#include "Game.h"
#include "Config.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>


Game::Game()
    : window(sf::VideoMode(1280, 720), "Tower Defense", sf::Style::Default),
      isFullscreen(false), playerSpeed(300.f), showPopup(false),
      selectedTower(nullptr), levelBtns(5), levelTexts(5), money(500),
      debugMode(false), baseHp(100), maxBaseHp(100), playerHp(50), maxPlayerHp(50) {
  window.setFramerateLimit(60);
  window.setView(sf::View(sf::FloatRect(0.f, 0.f, 1280.f, 720.f)));

  if (!map.loadFromFile("../../../assets/map/level1.map")) {
    std::cerr << "Failed to load level1.map\n";
  }

  assets.loadAllTextures();
  player.setTexture(assets.playerTexture);
  player.setTextureRect(sf::IntRect(0, 0, 100, 100));
  player.setOrigin(50.f, 50.f);
  player.setScale(1.3f, 1.3f);
  player.setPosition(400.f, 300.f);
  playerFrame = 0;
  playerAnimTime = 0.f;
  towerAnimTime = 0.f;
  towerAnimFrame = 0;

  Config::loadTowerConfig("../../../config/tower.config", towerConfigs);
  Config::loadEnemyConfig("../../../config/enemy.config", enemyConfigs);

  for (const auto &spot : map.getTowerSpots()) {
    towers.push_back({spot});
  }

  popupBg.setSize(sf::Vector2f(500.f, 350.f));
  popupBg.setFillColor(sf::Color(50, 50, 50, 240));
  popupBg.setOutlineThickness(2.f);
  popupBg.setOutlineColor(sf::Color::White);
  popupBg.setOrigin(250.f, 175.f);
  popupBg.setPosition(640.f, 360.f);

  closeBtn.setSize(sf::Vector2f(30.f, 30.f));
  closeBtn.setFillColor(sf::Color::Red);
  closeBtn.setOrigin(15.f, 15.f);
  closeBtn.setPosition(865.f, 210.f);

  cross1.setSize(sf::Vector2f(30.f, 4.f));
  cross1.setFillColor(sf::Color::White);
  cross1.setOrigin(15.f, 2.f);
  cross1.setPosition(865.f, 210.f);
  cross1.setRotation(45.f);

  cross2.setSize(sf::Vector2f(30.f, 4.f));
  cross2.setFillColor(sf::Color::White);
  cross2.setOrigin(15.f, 2.f);
  cross2.setPosition(865.f, 210.f);
  cross2.setRotation(-45.f);

  hasFont = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
  text.setFont(font);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::White);

  if (hasFont) {
    moneyText.setFont(font);
    moneyText.setCharacterSize(24);
    moneyText.setFillColor(sf::Color::Yellow);
    moneyText.setPosition(20.f, 20.f);
  }

  for (int i = 0; i < 5; ++i) {
    levelBtns[i].setSize(sf::Vector2f(40.f, 40.f));
    levelBtns[i].setFillColor(sf::Color(100, 100, 100));
    levelBtns[i].setOutlineThickness(2.f);
    levelBtns[i].setOutlineColor(sf::Color::White);
    levelBtns[i].setPosition(500.f + i * 60.f, 470.f);

    if (hasFont) {
      levelTexts[i].setFont(font);
      levelTexts[i].setString(std::to_string(i));
      levelTexts[i].setCharacterSize(20);
      levelTexts[i].setFillColor(sf::Color::White);
      sf::FloatRect b = levelTexts[i].getLocalBounds();
      levelTexts[i].setOrigin(b.width / 2.f, b.height / 2.f);
      levelTexts[i].setPosition(520.f + i * 60.f, 485.f);
    }
  }
}

void Game::run() {
  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    processEvents();
    update(dt);
    render();
  }
}

void Game::processEvents() {
  sf::Event event;
  const auto &pathPoints = map.getPathPoints();

  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      window.close();

    if (event.type == sf::Event::KeyPressed) {
      if (event.key.code == sf::Keyboard::F11) {
        isFullscreen = !isFullscreen;
        if (isFullscreen) {
          window.create(sf::VideoMode::getDesktopMode(), "Tower Defense",
                        sf::Style::Fullscreen);
        } else {
          window.create(sf::VideoMode(1280, 720), "Tower Defense",
                        sf::Style::Default);
        }
        window.setFramerateLimit(60);
        window.setView(sf::View(sf::FloatRect(0.f, 0.f, 1280.f, 720.f)));
      }

      if (event.key.code == sf::Keyboard::Z && event.key.control &&
          event.key.shift) {
        debugMode = !debugMode;
      }

      if (debugMode) {
        if (event.key.code == sf::Keyboard::P) {
          if (!pathPoints.empty() && !enemyConfigs.empty()) {
            auto it = enemyConfigs.begin();
            std::advance(it, rand() % enemyConfigs.size());
            EnemyStats stats = it->second;
            const std::string& ename = it->first;
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
            enemies.push_back(std::make_unique<Enemy>(pathPoints[0], stats, tD, tS, tU));
          }
        }

        if (event.key.code == sf::Keyboard::O) {
          for (auto &enemy : enemies) {
            enemy->takeDamage(5);
          }
        }

        if (event.key.code == sf::Keyboard::K) {
          money += 1000;
        }
      }

      if (event.key.code == sf::Keyboard::E && !showPopup) {
        float interactionDist = 60.f;
        selectedTower = nullptr;
        for (auto &t : towers) {
          float dx = player.getPosition().x - t.position.x;
          float dy = player.getPosition().y - t.position.y;
          if (std::sqrt(dx * dx + dy * dy) < interactionDist) {
            showPopup = true;
            selectedTower = &t;

            if (t.state == TowerState::Building ||
                t.state == TowerState::Upgrading) {
              popupText = "WIEZA W BUDOWIE...";
            } else {
              if (t.type == TowerType::Empty) {
                popupText = "BUDUJ WIEZE\n0 - Podstawowa (" +
                            std::to_string(towerConfigs[TowerType::Base].cost) +
                            "$)";
              } else {
                TowerStats stats = towerConfigs[t.type];
                int currDmg = stats.damage;
                for (int i = 0; i < t.damageUpgradeLevel && i < static_cast<int>(stats.damageUpgrades.size()); ++i)
                    currDmg += stats.damageUpgrades[i].second;
                
                float currSpd = stats.cooldown;
                for (int i = 0; i < t.speedUpgradeLevel && i < static_cast<int>(stats.speedUpgrades.size()); ++i)
                    currSpd -= stats.speedUpgrades[i].second;
                currSpd = std::max(0.1f, currSpd);

                std::string typeName =
                    (t.type == TowerType::Base)
                        ? "PODSTAWOWA"
                        : ((t.type == TowerType::Sniper) ? "SNAJPER"
                                                         : "MULTISHOT");

                char buf[64];
                snprintf(buf, sizeof(buf), "%.2fs", currSpd);
                
                std::string extraStats = "";
                if (t.type == TowerType::Multishot) {
                    int maxTgt = stats.baseTargets;
                    if (t.targetsUpgradeLevel > 0 && t.targetsUpgradeLevel <= static_cast<int>(stats.targetUpgrades.size())) {
                        maxTgt = stats.targetUpgrades[t.targetsUpgradeLevel - 1].second;
                    }
                    extraStats = " | Cele: " + std::to_string(maxTgt);
                }

                std::string speedUpgText = "MAX";
                if (t.speedUpgradeLevel < static_cast<int>(stats.speedUpgrades.size())) {
                    auto upg = stats.speedUpgrades[t.speedUpgradeLevel];
                    char buf2[64];
                    snprintf(buf2, sizeof(buf2), "%.2f", upg.second);
                    speedUpgText = "-" + std::string(buf2) + "s za " + std::to_string(upg.first) + "$";
                }

                std::string dmgUpgText = "MAX";
                if (t.damageUpgradeLevel < static_cast<int>(stats.damageUpgrades.size())) {
                    auto upg = stats.damageUpgrades[t.damageUpgradeLevel];
                    dmgUpgText = "+" + std::to_string(upg.second) + " za " + std::to_string(upg.first) + "$";
                }

                popupText =
                    "WIEZA " + typeName + (t.hasTimedUpgrade ? " (BOOST)" : "") + "\nObr: " + std::to_string(currDmg) +
                    " | Przelad: " + std::string(buf) + extraStats + "\n" +
                    "0 - Szybkosc (" + speedUpgText + ")\n" +
                    "1 - Obrazenia (" + dmgUpgText + ")";

                int nextBtnIndex = 2;
                if (t.type == TowerType::Base) {
                  bool isMaxUpgraded = (t.speedUpgradeLevel >= static_cast<int>(stats.speedUpgrades.size())) &&
                                       (t.damageUpgradeLevel >= static_cast<int>(stats.damageUpgrades.size()));
                  if (isMaxUpgraded) {
                    popupText += "\n2 - Ewolucja Snajper (" + std::to_string(towerConfigs[TowerType::Sniper].cost) + "$)\n" + 
                                 "3 - Ewolucja Multishot (" + std::to_string(towerConfigs[TowerType::Multishot].cost) + "$)";
                  } else {
                    popupText += "\n2 - Ewolucja Snajper (Wymagany MAX)\n" 
                                 "3 - Ewolucja Multishot (Wymagany MAX)";
                  }
                  nextBtnIndex = 4;
                } else if (t.type == TowerType::Multishot) {
                    if (t.targetsUpgradeLevel < static_cast<int>(stats.targetUpgrades.size())) {
                        int cost = stats.targetUpgrades[t.targetsUpgradeLevel].first;
                        int nextTgt = stats.targetUpgrades[t.targetsUpgradeLevel].second;
                        popupText += "\n2 - Dodatkowy cel (Celow: " + std::to_string(nextTgt) + " za " + std::to_string(cost) + "$)";
                    } else {
                        popupText += "\n2 - Dodatkowy cel (MAX)";
                    }
                    nextBtnIndex = 3;
                }

                popupText += "\n" + std::to_string(nextBtnIndex) + " - Ulepszenie Czasowe (" + std::to_string(stats.timedUpgrade.cost) + "$)";
              }
            }
            break;
          }
        }
        if (!showPopup) {
          for (const auto &spot : map.getStoreSpots()) {
            float dx = player.getPosition().x - spot.x;
            float dy = player.getPosition().y - spot.y;
            if (std::sqrt(dx * dx + dy * dy) < interactionDist) {
              showPopup = true;
              popupText = "MENU SKLEPU";
              break;
            }
          }
        }
        if (showPopup && hasFont) {
          text.setString(popupText);
          sf::FloatRect bounds = text.getLocalBounds();
          text.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
          text.setPosition(640.f, 320.f);
        }
      }
    }

    if (event.type == sf::Event::MouseButtonPressed && showPopup) {
      if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        handlePopupClick(worldPos.x, worldPos.y);
      }
    }
  }
}

void Game::handlePopupClick(float mx, float my) {
  if (mx >= 850.f && mx <= 880.f && my >= 195.f && my <= 225.f) {
    showPopup = false;
  }
  if (selectedTower && selectedTower->state == TowerState::Idle) {
    for (int i = 0; i < 5; ++i) {
      if (mx >= 500.f + i * 60.f && mx <= 540.f + i * 60.f && my >= 470.f &&
          my <= 510.f) {
        if (selectedTower->type == TowerType::Empty && i == 0) {
          int cost = towerConfigs[TowerType::Base].cost;
          if (money >= cost) {
            money -= cost;
            selectedTower->type = TowerType::Base;
            selectedTower->state = TowerState::Building;
            selectedTower->buildTimer = towerConfigs[TowerType::Base].buildTime;
            showPopup = false;
          }
        } else if (selectedTower->type != TowerType::Empty) {
          TowerStats stats = towerConfigs[selectedTower->type];
          
          int timedBtnIndex = 2;
          if (selectedTower->type == TowerType::Base) timedBtnIndex = 4;
          else if (selectedTower->type == TowerType::Multishot) timedBtnIndex = 3;

          if (i == 0) { // Speed upgrade
            if (selectedTower->speedUpgradeLevel < static_cast<int>(stats.speedUpgrades.size())) {
                int cost = stats.speedUpgrades[selectedTower->speedUpgradeLevel].first;
                if (money >= cost) {
                  money -= cost;
                  selectedTower->speedUpgradeLevel++;
                  showPopup = false;
                }
            }
          } else if (i == 1) { // Damage upgrade
            if (selectedTower->damageUpgradeLevel < static_cast<int>(stats.damageUpgrades.size())) {
                int cost = stats.damageUpgrades[selectedTower->damageUpgradeLevel].first;
                if (money >= cost) {
                  money -= cost;
                  selectedTower->damageUpgradeLevel++;
                  showPopup = false;
                }
            }
          } else if (selectedTower->type == TowerType::Multishot && i == 2) {
              if (selectedTower->targetsUpgradeLevel < static_cast<int>(stats.targetUpgrades.size())) {
                  int cost = stats.targetUpgrades[selectedTower->targetsUpgradeLevel].first;
                  if (money >= cost) {
                      money -= cost;
                      selectedTower->targetsUpgradeLevel++;
                      showPopup = false;
                  }
              }
          } else if (selectedTower->type == TowerType::Base) {
            bool isMaxUpgraded = (selectedTower->speedUpgradeLevel >= static_cast<int>(stats.speedUpgrades.size())) &&
                                 (selectedTower->damageUpgradeLevel >= static_cast<int>(stats.damageUpgrades.size()));
            if (isMaxUpgraded) {
              if (i == 2) { // Sniper
                int cost = towerConfigs[TowerType::Sniper].cost;
                if (money >= cost) {
                  money -= cost;
                  selectedTower->type = TowerType::Sniper;
                  selectedTower->state = TowerState::Upgrading;
                  selectedTower->buildTimer = towerConfigs[TowerType::Sniper].buildTime;
                  selectedTower->damageUpgradeLevel = 0;
                  selectedTower->speedUpgradeLevel = 0;
                  selectedTower->hasTimedUpgrade = false;
                  selectedTower->timedUpgradeTimer = 0.f;
                  showPopup = false;
                }
              } else if (i == 3) { // Multishot
                int cost = towerConfigs[TowerType::Multishot].cost;
                if (money >= cost) {
                  money -= cost;
                  selectedTower->type = TowerType::Multishot;
                  selectedTower->state = TowerState::Upgrading;
                  selectedTower->buildTimer = towerConfigs[TowerType::Multishot].buildTime;
                  selectedTower->damageUpgradeLevel = 0;
                  selectedTower->speedUpgradeLevel = 0;
                  selectedTower->targetsUpgradeLevel = 0;
                  selectedTower->hasTimedUpgrade = false;
                  selectedTower->timedUpgradeTimer = 0.f;
                  showPopup = false;
                }
              }
            }
          }

          if (i == timedBtnIndex) {
              int cost = stats.timedUpgrade.cost;
              if (money >= cost) {
                  money -= cost;
                  selectedTower->hasTimedUpgrade = true;
                  selectedTower->timedUpgradeTimer = stats.timedUpgrade.duration;
                  showPopup = false;
              }
          }
        }
      }
    }
  }
}

void Game::update(float dt) {
  if (!showPopup) {
    sf::Vector2f playerMove(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
      playerMove.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
      playerMove.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
      playerMove.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
      playerMove.x += 1.f;

    if (playerMove.x != 0.f || playerMove.y != 0.f) {
      float len =
          std::sqrt(playerMove.x * playerMove.x + playerMove.y * playerMove.y);
      playerMove /= len;

      playerAnimTime += dt;
      if (playerAnimTime >= 0.1f) {
        playerAnimTime = 0.f;
        playerFrame = (playerFrame + 1) % 8;
        player.setTextureRect(sf::IntRect(playerFrame * 100, 0, 100, 100));
      }
      if (playerMove.x < 0) {
        player.setScale(-1.3f, 1.3f);
      } else if (playerMove.x > 0) {
        player.setScale(1.3f, 1.3f);
      }
    } else {
      playerFrame = 0;
      player.setTextureRect(sf::IntRect(0, 0, 100, 100));
    }
    player.move(playerMove * playerSpeed * dt);

    sf::Vector2f pos = player.getPosition();
    if (pos.x < 20.f)
      pos.x = 20.f;
    if (pos.x > 1260.f)
      pos.x = 1260.f;
    if (pos.y < 20.f)
      pos.y = 20.f;
    if (pos.y > 700.f)
      pos.y = 700.f;
    player.setPosition(pos);
  }

  const auto &pathPoints = map.getPathPoints();

  towerAnimTime += dt;
  if (towerAnimTime >= 0.15f) {
    towerAnimTime = 0.f;
    towerAnimFrame = (towerAnimFrame + 1) % 4;
  }

  for (auto &t : towers) {
    if (t.hasTimedUpgrade) {
      t.timedUpgradeTimer -= dt;
      if (t.timedUpgradeTimer <= 0.f) {
        t.hasTimedUpgrade = false;
      }
    }

    if (t.state == TowerState::Building || t.state == TowerState::Upgrading) {
      t.buildTimer -= dt;
      if (t.buildTimer <= 0.f) {
        t.state = TowerState::Idle;
      }
    } else if (t.state == TowerState::Idle && t.type != TowerType::Empty) {
      t.cooldownTimer -= dt;
      if (t.cooldownTimer <= 0.f) {
        TowerStats stats = towerConfigs[t.type];
        float range = stats.range;
        
        int damage = stats.damage;
        for (int i = 0; i < t.damageUpgradeLevel && i < static_cast<int>(stats.damageUpgrades.size()); ++i) {
            damage += stats.damageUpgrades[i].second;
        }

        float cooldown = stats.cooldown;
        for (int i = 0; i < t.speedUpgradeLevel && i < static_cast<int>(stats.speedUpgrades.size()); ++i) {
            cooldown -= stats.speedUpgrades[i].second;
        }
        float maxCooldown = std::max(0.1f, cooldown);

        if (t.hasTimedUpgrade) {
            damage = static_cast<int>(damage * stats.timedUpgrade.damageMultiplier);
            maxCooldown *= stats.timedUpgrade.cooldownMultiplier;
            maxCooldown = std::max(0.01f, maxCooldown);
        }

        if (t.type == TowerType::Base || t.type == TowerType::Sniper) {
          for (auto &enemy : enemies) {
            if (enemy->isDead())
              continue;
            sf::Vector2f ePos = enemy->getPosition();
            float dx = ePos.x - t.position.x;
            float dy = ePos.y - t.position.y;
            if (std::sqrt(dx * dx + dy * dy) <= range) {
              enemy->takeDamage(damage);
              t.cooldownTimer = maxCooldown;
              break;
            }
          }
        } else if (t.type == TowerType::Multishot) {
          int maxTargets = stats.baseTargets;
          if (t.targetsUpgradeLevel > 0 && t.targetsUpgradeLevel <= stats.targetUpgrades.size()) {
              maxTargets = stats.targetUpgrades[t.targetsUpgradeLevel - 1].second;
          }
          
          std::vector<std::pair<float, Enemy*>> targetsInRange;
          for (auto &enemy : enemies) {
            if (enemy->isDead())
              continue;
            sf::Vector2f ePos = enemy->getPosition();
            float dx = ePos.x - t.position.x;
            float dy = ePos.y - t.position.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= range) {
              targetsInRange.push_back({dist, enemy.get()});
            }
          }
          
          if (!targetsInRange.empty()) {
              std::sort(targetsInRange.begin(), targetsInRange.end());
              int targetsHit = 0;
              for (auto &pair : targetsInRange) {
                  if (targetsHit >= maxTargets) break;
                  pair.second->takeDamage(damage);
                  targetsHit++;
              }
              t.cooldownTimer = maxCooldown;
          }
        }
      }
    }
  }

  for (auto it = enemies.begin(); it != enemies.end();) {
    (*it)->update(dt, pathPoints);
    
    sf::Vector2f ePos = (*it)->getPosition();
    sf::Vector2f pPos = player.getPosition();
    float dx = ePos.x - pPos.x;
    float dy = ePos.y - pPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    bool hitPlayer = false;
    
    if (dist < 40.f && !(*it)->isDead()) {
        playerHp -= (*it)->getPlayerDamage();
        if (playerHp < 0) playerHp = 0;
        (*it)->die();
        hitPlayer = true;
    }

    if ((*it)->hasReachedEnd(pathPoints.size()) || (*it)->isDead()) {
      if ((*it)->hasReachedEnd(pathPoints.size())) {
        baseHp -= (*it)->getCastleDamage();
        if (baseHp < 0) baseHp = 0;
      } else if (!hitPlayer) {
        money += 10;
      }
      it = enemies.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::render() {
  window.clear();

  sf::Vector2i mousePos = sf::Mouse::getPosition(window);
  sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
  
  hoveredTower = nullptr;
  if (!showPopup) {
      for (auto &t : towers) {
          if (t.type != TowerType::Empty) {
              float dx = worldPos.x - t.position.x;
              float dy = worldPos.y - t.position.y;
              if (std::sqrt(dx * dx + dy * dy) < 35.f) {
                  hoveredTower = &t;
                  break;
              }
          }
      }
  }

  map.draw(window);

  for (const auto &t : towers) {
    sf::Sprite towerSprite;
    int fw = 70, fh = 130;
    
    if (t.type == TowerType::Empty) {
      towerSprite.setTexture(assets.towerUpgradeTexs[0]);
      towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
    } else {
      sf::Texture* activeTex = nullptr;
      int frameIndex = 0;

      if (t.type == TowerType::Base) {
          activeTex = &assets.towerUpgradeTexs[3]; // 4upgrade.png
          bool isMax = (t.speedUpgradeLevel >= 3 && t.damageUpgradeLevel >= 3);
          bool isLvl2 = (!isMax && t.speedUpgradeLevel + t.damageUpgradeLevel >= 4);
          bool isLvl1 = (!isMax && !isLvl2 && t.speedUpgradeLevel + t.damageUpgradeLevel >= 2);
          if (isMax) frameIndex = 3;
          else if (isLvl2) frameIndex = 2;
          else if (isLvl1) frameIndex = 1;
          else frameIndex = 0;
      } else if (t.type == TowerType::Sniper) {
          bool isUpgraded = (t.speedUpgradeLevel > 0 || t.damageUpgradeLevel > 0);
          if (isUpgraded) {
              activeTex = &assets.towerUpgradeTexs[6]; // 7upgrade.png
              frameIndex = 3; // Ostatnia forma z 7upgrade
          } else {
              activeTex = &assets.towerUpgradeTexs[4]; // 5upgrade.png
              frameIndex = 0; 
          }
      } else if (t.type == TowerType::Multishot) {
          activeTex = &assets.towerUpgradeTexs[5]; // 6upgrade.png
          bool isUpgraded = (t.speedUpgradeLevel > 0 || t.damageUpgradeLevel > 0 || t.targetsUpgradeLevel > 0);
          if (isUpgraded) frameIndex = 3; // Ostatnia tekstura z pliku 6upgrade.png
          else frameIndex = 0; // Pierwsza tekstura
      }

      towerSprite.setTexture(*activeTex);
      towerSprite.setTextureRect(sf::IntRect(frameIndex * fw, 0, fw, fh));
    }
    
    towerSprite.setOrigin(fw / 2.f, fh / 2.f + 15.f);
    towerSprite.setPosition(t.position);

    window.draw(towerSprite);

    // Draw building progress bar
    if (t.state == TowerState::Building || t.state == TowerState::Upgrading) {
      float maxTimer = 1.f;
      if (t.type == TowerType::Base)
        maxTimer = towerConfigs[TowerType::Base].buildTime;
      else if (t.type == TowerType::Sniper)
        maxTimer = towerConfigs[TowerType::Sniper].buildTime;
      else if (t.type == TowerType::Multishot)
        maxTimer = towerConfigs[TowerType::Multishot].buildTime;

      float progress = 1.f - (t.buildTimer / maxTimer);
      if (progress < 0.f)
        progress = 0.f;

      sf::RectangleShape bgBar(sf::Vector2f(40.f, 5.f));
      bgBar.setFillColor(sf::Color::Red);
      bgBar.setOrigin(20.f, 2.5f);
      bgBar.setPosition(t.position.x, t.position.y - 40.f);

      sf::RectangleShape fgBar(sf::Vector2f(40.f * progress, 5.f));
      fgBar.setFillColor(sf::Color::Blue);
      fgBar.setOrigin(20.f, 2.5f);
      fgBar.setPosition(t.position.x, t.position.y - 40.f);

      window.draw(bgBar);
      window.draw(fgBar);
    }

    if (t.type != TowerType::Empty && t.state == TowerState::Idle) {
      // Draw damage upgrade indicators (red squares/dots)
      for (int i = 0; i < t.damageUpgradeLevel; ++i) {
        sf::RectangleShape dmgDot(sf::Vector2f(6.f, 6.f));
        dmgDot.setFillColor(sf::Color::Red);
        dmgDot.setOutlineThickness(1.f);
        dmgDot.setOutlineColor(sf::Color::Black);
        dmgDot.setOrigin(3.f, 3.f);
        dmgDot.setPosition(t.position.x - 20.f + i * 8.f, t.position.y + 30.f);
        window.draw(dmgDot);
      }
      
      // Draw speed upgrade indicators (cyan squares/dots)
      for (int i = 0; i < t.speedUpgradeLevel; ++i) {
        sf::RectangleShape spdDot(sf::Vector2f(6.f, 6.f));
        spdDot.setFillColor(sf::Color::Cyan);
        spdDot.setOutlineThickness(1.f);
        spdDot.setOutlineColor(sf::Color::Black);
        spdDot.setOrigin(3.f, 3.f);
        spdDot.setPosition(t.position.x + 5.f + i * 8.f, t.position.y + 30.f);
        window.draw(spdDot);
      }
      
      // Draw timed upgrade bar (yellow progress bar)
      if (t.hasTimedUpgrade) {
        float maxTimer = towerConfigs[t.type].timedUpgrade.duration;
        float progress = t.timedUpgradeTimer / maxTimer;
        if (progress < 0.f) progress = 0.f;

        sf::RectangleShape bgBar(sf::Vector2f(40.f, 4.f));
        bgBar.setFillColor(sf::Color(50, 50, 50));
        bgBar.setOutlineThickness(1.f);
        bgBar.setOutlineColor(sf::Color::Black);
        bgBar.setOrigin(20.f, 2.f);
        bgBar.setPosition(t.position.x, t.position.y - 55.f);

        sf::RectangleShape fgBar(sf::Vector2f(40.f * progress, 4.f));
        fgBar.setFillColor(sf::Color::Yellow);
        fgBar.setOrigin(20.f, 2.f);
        fgBar.setPosition(t.position.x, t.position.y - 55.f);

        window.draw(bgBar);
        window.draw(fgBar);
      }
    }

    if (t.type != TowerType::Empty && t.state == TowerState::Idle) {
      float range = towerConfigs[t.type].range;
      sf::CircleShape rangeCircle(range);
      rangeCircle.setOrigin(range, range);
      rangeCircle.setPosition(t.position);
      rangeCircle.setFillColor(sf::Color(100, 100, 100, 50));
      rangeCircle.setOutlineThickness(1.f);
      rangeCircle.setOutlineColor(sf::Color(150, 150, 150, 100));
      window.draw(rangeCircle);

      if (t.type == TowerType::Base || t.type == TowerType::Sniper) {
        for (const auto &enemy : enemies) {
          sf::Vector2f ePos = enemy->getPosition();
          float dx = ePos.x - t.position.x;
          float dy = ePos.y - t.position.y;
          if (std::sqrt(dx * dx + dy * dy) <= range) {
            sf::Vertex line[] = {sf::Vertex(t.position, sf::Color::Yellow),
                                 sf::Vertex(ePos, sf::Color::Yellow)};
            window.draw(line, 2, sf::Lines);
            break;
          }
        }
      } else if (t.type == TowerType::Multishot) {
          int maxTargets = towerConfigs[TowerType::Multishot].baseTargets;
          if (t.targetsUpgradeLevel > 0 && t.targetsUpgradeLevel <= towerConfigs[TowerType::Multishot].targetUpgrades.size()) {
              maxTargets = towerConfigs[TowerType::Multishot].targetUpgrades[t.targetsUpgradeLevel - 1].second;
          }
          
          std::vector<std::pair<float, const Enemy*>> targetsInRange;
          for (const auto &enemy : enemies) {
            if (enemy->isDead()) continue;
            sf::Vector2f ePos = enemy->getPosition();
            float dx = ePos.x - t.position.x;
            float dy = ePos.y - t.position.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= range) {
              targetsInRange.push_back({dist, enemy.get()});
            }
          }
          
          std::sort(targetsInRange.begin(), targetsInRange.end());
          int targetsHit = 0;
          for (auto &pair : targetsInRange) {
              if (targetsHit >= maxTargets) break;
              sf::Vertex line[] = {sf::Vertex(t.position, sf::Color::Cyan),
                                   sf::Vertex(pair.second->getPosition(), sf::Color::Cyan)};
              window.draw(line, 2, sf::Lines);
              targetsHit++;
          }
      }
    }
  }

  for (const auto &enemy : enemies) {
    enemy->draw(window);
  }
  window.draw(player);

  if (playerHp < maxPlayerHp) {
      sf::RectangleShape pBg(sf::Vector2f(40.f, 5.f));
      pBg.setFillColor(sf::Color::Red);
      pBg.setOrigin(20.f, 2.5f);
      pBg.setPosition(player.getPosition().x, player.getPosition().y - 45.f);

      float pProg = static_cast<float>(playerHp) / maxPlayerHp;
      if (pProg < 0) pProg = 0;
      sf::RectangleShape pFg(sf::Vector2f(40.f * pProg, 5.f));
      pFg.setFillColor(sf::Color::Green);
      pFg.setOrigin(20.f, 2.5f);
      pFg.setPosition(player.getPosition().x, player.getPosition().y - 45.f);

      window.draw(pBg);
      window.draw(pFg);
  }

  sf::RectangleShape bBg(sf::Vector2f(400.f, 20.f));
  bBg.setFillColor(sf::Color(100, 0, 0));
  bBg.setOutlineThickness(2.f);
  bBg.setOutlineColor(sf::Color::White);
  bBg.setOrigin(200.f, 0.f);
  bBg.setPosition(1280.f / 2.f, 10.f);

  float bProg = static_cast<float>(baseHp) / maxBaseHp;
  if (bProg < 0) bProg = 0;
  sf::RectangleShape bFg(sf::Vector2f(400.f * bProg, 20.f));
  bFg.setFillColor(sf::Color::Green);
  bFg.setOrigin(200.f, 0.f);
  bFg.setPosition(1280.f / 2.f, 10.f);

  window.draw(bBg);
  window.draw(bFg);

  if (hasFont) {
    moneyText.setString("Pieniadze: " + std::to_string(money));
    window.draw(moneyText);
  }

  if (showPopup) {
    window.draw(popupBg);
    window.draw(closeBtn);
    window.draw(cross1);
    window.draw(cross2);
    if (hasFont)
      window.draw(text);
    if (selectedTower) {
      int numBtns = 0;
      if (selectedTower->state == TowerState::Building ||
          selectedTower->state == TowerState::Upgrading) {
        numBtns = 0; // No buttons during build
      } else if (selectedTower->type == TowerType::Empty) {
        numBtns = 1; // 0: Build Base
      } else if (selectedTower->type == TowerType::Base) {
        numBtns = 5; // 0: Speed, 1: Dmg, 2: Sniper, 3: Multishot, 4: Timed
      } else if (selectedTower->type == TowerType::Multishot) {
        numBtns = 4; // 0: Speed, 1: Dmg, 2: Targets, 3: Timed
      } else {
        numBtns = 3; // 0: Speed, 1: Damage, 2: Timed
      }

      for (int i = 0; i < numBtns; ++i) {
        if (i >= 5)
          continue;

        int cost = 0;
        bool isMax = false;
        if (selectedTower->type == TowerType::Empty && i == 0)
          cost = towerConfigs[TowerType::Base].cost;
        else if (selectedTower->type != TowerType::Empty) {
          TowerStats stats = towerConfigs[selectedTower->type];
          
          int timedBtnIndex = 2;
          if (selectedTower->type == TowerType::Base) timedBtnIndex = 4;
          else if (selectedTower->type == TowerType::Multishot) timedBtnIndex = 3;

          if (i == 0) {
              if (selectedTower->speedUpgradeLevel < static_cast<int>(stats.speedUpgrades.size()))
                  cost = stats.speedUpgrades[selectedTower->speedUpgradeLevel].first;
              else
                  isMax = true;
          } else if (i == 1) {
              if (selectedTower->damageUpgradeLevel < static_cast<int>(stats.damageUpgrades.size()))
                  cost = stats.damageUpgrades[selectedTower->damageUpgradeLevel].first;
              else
                  isMax = true;
          } else if (selectedTower->type == TowerType::Base) {
            bool isMaxUpgraded = (selectedTower->speedUpgradeLevel >= static_cast<int>(stats.speedUpgrades.size())) &&
                                 (selectedTower->damageUpgradeLevel >= static_cast<int>(stats.damageUpgrades.size()));
            if (i == 2) {
              cost = towerConfigs[TowerType::Sniper].cost;
              if (!isMaxUpgraded) cost = 9999999;
            } else if (i == 3) {
              cost = towerConfigs[TowerType::Multishot].cost;
              if (!isMaxUpgraded) cost = 9999999;
            }
          } else if (selectedTower->type == TowerType::Multishot && i == 2) {
            if (selectedTower->targetsUpgradeLevel < static_cast<int>(stats.targetUpgrades.size())) {
                cost = stats.targetUpgrades[selectedTower->targetsUpgradeLevel].first;
            } else {
                isMax = true;
            }
          }
          
          if (i == timedBtnIndex) {
              cost = stats.timedUpgrade.cost;
          }
        }

        if (isMax) {
          levelBtns[i].setFillColor(sf::Color(50, 150, 50)); // Green tint
        } else if (money >= cost) {
          levelBtns[i].setFillColor(sf::Color(100, 100, 100));
        } else {
          levelBtns[i].setFillColor(sf::Color(150, 50, 50)); // Red tint
        }

        window.draw(levelBtns[i]);
        if (hasFont)
          window.draw(levelTexts[i]);
      }
    }
  }

  if (hoveredTower && !showPopup && hasFont) {
      TowerStats stats = towerConfigs[hoveredTower->type];
      int currentDamage = stats.damage;
      if (hoveredTower->damageUpgradeLevel > 0 && hoveredTower->damageUpgradeLevel <= static_cast<int>(stats.damageUpgrades.size())) {
          currentDamage = stats.damageUpgrades[hoveredTower->damageUpgradeLevel - 1].second;
      }
      float currentReload = stats.cooldown;
      if (hoveredTower->speedUpgradeLevel > 0 && hoveredTower->speedUpgradeLevel <= static_cast<int>(stats.speedUpgrades.size())) {
          currentReload = stats.speedUpgrades[hoveredTower->speedUpgradeLevel - 1].second;
      }
      
      std::string tName = "WIEZA";
      if (hoveredTower->type == TowerType::Base) tName = "Podstawowa";
      else if (hoveredTower->type == TowerType::Sniper) tName = "Snajper";
      else if (hoveredTower->type == TowerType::Multishot) tName = "Wielostrzal";
      
      std::string hoverStr = tName + "\nObrazenia: " + std::to_string(currentDamage) + "\nPrzeladowanie: " + std::to_string(currentReload).substr(0, 4) + "s";
      
      sf::Text hoverText(hoverStr, font, 14);
      hoverText.setFillColor(sf::Color::White);
      sf::FloatRect textBounds = hoverText.getLocalBounds();
      
      sf::RectangleShape hoverBg(sf::Vector2f(textBounds.width + 10.f, textBounds.height + 10.f));
      hoverBg.setFillColor(sf::Color(0, 0, 0, 200));
      hoverBg.setOutlineThickness(1.f);
      hoverBg.setOutlineColor(sf::Color::White);
      
      float tooltipX = worldPos.x + 15.f;
      float tooltipY = worldPos.y + 15.f;
      
      if (tooltipX + hoverBg.getSize().x > 1280.f) tooltipX = worldPos.x - hoverBg.getSize().x - 15.f;
      if (tooltipY + hoverBg.getSize().y > 720.f) tooltipY = worldPos.y - hoverBg.getSize().y - 15.f;
      
      hoverBg.setPosition(tooltipX, tooltipY);
      hoverText.setPosition(tooltipX + 5.f, tooltipY + 5.f);
      
      window.draw(hoverBg);
      window.draw(hoverText);
  }

  window.display();
}
