#include "Game.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>


Game::Game()
    : window(sf::VideoMode(1280, 720), "Tower Defense", sf::Style::Default),
      isFullscreen(false), playerSpeed(300.f), showPopup(false),
      selectedTower(nullptr), levelBtns(4), levelTexts(4), money(500),
      debugMode(false) {
  window.setFramerateLimit(60);
  window.setView(sf::View(sf::FloatRect(0.f, 0.f, 1280.f, 720.f)));

  if (!map.loadFromFile("../../../map/level1.map")) {
    std::cerr << "Nie udalo sie wczytac mapy z zadnej sciezki wzglednej!\n";
  }

  playerTexture.loadFromFile("../../../textures/Soldier-Walk.png");
  player.setTexture(playerTexture);
  player.setTextureRect(sf::IntRect(0, 0, 100, 100));
  player.setOrigin(50.f, 50.f);
  player.setScale(1.3f, 1.3f);
  player.setPosition(400.f, 300.f);
  playerFrame = 0;
  playerAnimTime = 0.f;

  enemyTexD.loadFromFile("../../../textures/D_Walk.png");
  enemyTexS.loadFromFile("../../../textures/S_Walk.png");
  enemyTexU.loadFromFile("../../../textures/U_Walk.png");

  towerUnbuiltTex.loadFromFile("../../../textures/1upgrade.png");
  towerLvl1Tex.loadFromFile("../../../textures/1idle.png");
  towerLvl2Tex.loadFromFile("../../../textures/2idle.png");
  towerLvl3Tex.loadFromFile("../../../textures/3idle.png");
  towerAnimTime = 0.f;
  towerAnimFrame = 0;

  loadTowerConfig("../../../config/tower.config");

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

  for (int i = 0; i < 4; ++i) {
    levelBtns[i].setSize(sf::Vector2f(40.f, 40.f));
    levelBtns[i].setFillColor(sf::Color(100, 100, 100));
    levelBtns[i].setOutlineThickness(2.f);
    levelBtns[i].setOutlineColor(sf::Color::White);
    levelBtns[i].setPosition(530.f + i * 60.f, 470.f);

    if (hasFont) {
      levelTexts[i].setFont(font);
      levelTexts[i].setString(std::to_string(i));
      levelTexts[i].setCharacterSize(20);
      levelTexts[i].setFillColor(sf::Color::White);
      sf::FloatRect b = levelTexts[i].getLocalBounds();
      levelTexts[i].setOrigin(b.width / 2.f, b.height / 2.f);
      levelTexts[i].setPosition(550.f + i * 60.f, 485.f);
    }
  }
}

void Game::loadTowerConfig(const std::string &filepath) {
  // defaults
  towerConfigs[TowerType::Base] = {50, 3.0f, 5, 1.0f, 100.f, 30, 2, 25, 0.1f};
  towerConfigs[TowerType::Sniper] = {100, 5.0f, 25, 2.5f, 250.f,
                                     50,  5,    50, 0.2f};
  towerConfigs[TowerType::Multishot] = {120, 4.0f, 8,  0.8f, 120.f,
                                        40,  3,    30, 0.05f};

  std::ifstream file(filepath);
  if (!file.is_open()) {
    std::cerr << "Nie udalo sie wczytac pliku konfiguracyjnego wiez: "
              << filepath << "\n";
    return;
  }

  std::string line;
  TowerType currentType = TowerType::Empty;
  while (std::getline(file, line)) {
    size_t start = line.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
      continue;
    line = line.substr(start);

    if (line[0] == '#' || line.empty())
      continue;

    if (line.find("Tower_Base:") != std::string::npos) {
      currentType = TowerType::Base;
      continue;
    }
    if (line.find("Tower_Sniper:") != std::string::npos) {
      currentType = TowerType::Sniper;
      continue;
    }
    if (line.find("Tower_Multishot:") != std::string::npos) {
      currentType = TowerType::Multishot;
      continue;
    }

    if (currentType != TowerType::Empty) {
      size_t colonPos = line.find(':');
      if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        std::string valStr = line.substr(colonPos + 1);
        size_t kEnd = key.find_last_not_of(" \t");
        if (kEnd != std::string::npos)
          key = key.substr(0, kEnd + 1);
        size_t vStart = valStr.find_first_not_of(" \t");
        if (vStart != std::string::npos) {
          valStr = valStr.substr(vStart);
          size_t vEnd = valStr.find_last_not_of(" \t\r\n");
          if (vEnd != std::string::npos)
            valStr = valStr.substr(0, vEnd + 1);

          float val = std::stof(valStr);
          if (key == "cost")
            towerConfigs[currentType].cost = static_cast<int>(val);
          else if (key == "build_time")
            towerConfigs[currentType].buildTime = val;
          else if (key == "damage")
            towerConfigs[currentType].damage = static_cast<int>(val);
          else if (key == "cooldown")
            towerConfigs[currentType].cooldown = val;
          else if (key == "range")
            towerConfigs[currentType].range = val;
          else if (key == "upgrade_damage_cost")
            towerConfigs[currentType].upgDamageCost = static_cast<int>(val);
          else if (key == "upgrade_damage_amount")
            towerConfigs[currentType].upgDamageAmount = static_cast<int>(val);
          else if (key == "upgrade_speed_cost")
            towerConfigs[currentType].upgSpeedCost = static_cast<int>(val);
          else if (key == "upgrade_speed_amount")
            towerConfigs[currentType].upgSpeedAmount = val;
          else if (key == "targets_base")
            towerConfigs[currentType].baseTargets = static_cast<int>(val);
          else if (key.find("targets_upg_cost_") == 0) {
            try {
              int idx = std::stoi(key.substr(17)) - 1;
              if (idx >= 0) {
                  if (towerConfigs[currentType].targetUpgrades.size() <= idx) {
                      towerConfigs[currentType].targetUpgrades.resize(idx + 1, {0, 0});
                  }
                  towerConfigs[currentType].targetUpgrades[idx].first = static_cast<int>(val);
              }
            } catch (...) {}
          }
          else if (key.find("targets_upg_val_") == 0) {
            try {
              int idx = std::stoi(key.substr(16)) - 1;
              if (idx >= 0) {
                  if (towerConfigs[currentType].targetUpgrades.size() <= idx) {
                      towerConfigs[currentType].targetUpgrades.resize(idx + 1, {0, 0});
                  }
                  towerConfigs[currentType].targetUpgrades[idx].second = static_cast<int>(val);
              }
            } catch (...) {}
          }
        }
      }
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
          if (!pathPoints.empty()) {
            enemies.push_back(std::make_unique<Enemy>(pathPoints[0], &enemyTexD,
                                                      &enemyTexS, &enemyTexU));
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
                int currDmg = stats.damage +
                              (t.damageUpgradeLevel * stats.upgDamageAmount);
                float currSpd =
                    std::max(0.1f, stats.cooldown - (t.speedUpgradeLevel *
                                                     stats.upgSpeedAmount));

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
                    if (t.targetsUpgradeLevel > 0 && t.targetsUpgradeLevel <= stats.targetUpgrades.size()) {
                        maxTgt = stats.targetUpgrades[t.targetsUpgradeLevel - 1].second;
                    }
                    extraStats = " | Cele: " + std::to_string(maxTgt);
                }

                popupText =
                    "WIEZA " + typeName + "\nObr: " + std::to_string(currDmg) +
                    " | Przelad: " + std::string(buf) + extraStats + "\n" +
                    "0 - Szybkosc (-" +
                    std::to_string(stats.upgSpeedAmount).substr(0, 4) +
                    "s za " + std::to_string(stats.upgSpeedCost) + "$)\n" +
                    "1 - Obrazenia (+" + std::to_string(stats.upgDamageAmount) +
                    " za " + std::to_string(stats.upgDamageCost) + "$)";

                if (t.type == TowerType::Base) {
                  popupText +=
                      "\n2 - Ewolucja Snajper (" +
                      std::to_string(towerConfigs[TowerType::Sniper].cost) +
                      "$)\n" + "3 - Ewolucja Multishot (" +
                      std::to_string(towerConfigs[TowerType::Multishot].cost) +
                      "$)";
                } else if (t.type == TowerType::Multishot) {
                    if (t.targetsUpgradeLevel < stats.targetUpgrades.size()) {
                        int cost = stats.targetUpgrades[t.targetsUpgradeLevel].first;
                        int nextTgt = stats.targetUpgrades[t.targetsUpgradeLevel].second;
                        popupText += "\n2 - Dodatkowy cel (Celow: " + std::to_string(nextTgt) + " za " + std::to_string(cost) + "$)";
                    } else {
                        popupText += "\n2 - Dodatkowy cel (MAX)";
                    }
                }
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
    for (int i = 0; i < 4; ++i) {
      if (mx >= 530.f + i * 60.f && mx <= 570.f + i * 60.f && my >= 470.f &&
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
          if (i == 0) { // Speed upgrade
            int cost = stats.upgSpeedCost;
            if (money >= cost) {
              money -= cost;
              selectedTower->speedUpgradeLevel++;
              showPopup = false;
            }
          } else if (i == 1) { // Damage upgrade
            int cost = stats.upgDamageCost;
            if (money >= cost) {
              money -= cost;
              selectedTower->damageUpgradeLevel++;
              showPopup = false;
            }
          } else if (selectedTower->type == TowerType::Multishot && i == 2) {
              if (selectedTower->targetsUpgradeLevel < stats.targetUpgrades.size()) {
                  int cost = stats.targetUpgrades[selectedTower->targetsUpgradeLevel].first;
                  if (money >= cost) {
                      money -= cost;
                      selectedTower->targetsUpgradeLevel++;
                      showPopup = false;
                  }
              }
          } else if (selectedTower->type == TowerType::Base) {
            if (i == 2) { // Sniper
              int cost = towerConfigs[TowerType::Sniper].cost;
              if (money >= cost) {
                money -= cost;
                selectedTower->type = TowerType::Sniper;
                selectedTower->state = TowerState::Upgrading;
                selectedTower->buildTimer =
                    towerConfigs[TowerType::Sniper].buildTime;
                selectedTower->damageUpgradeLevel =
                    0; // reset upgrades on evolution
                selectedTower->speedUpgradeLevel = 0;
                showPopup = false;
              }
            } else if (i == 3) { // Multishot
              int cost = towerConfigs[TowerType::Multishot].cost;
              if (money >= cost) {
                money -= cost;
                selectedTower->type = TowerType::Multishot;
                selectedTower->state = TowerState::Upgrading;
                selectedTower->buildTimer =
                    towerConfigs[TowerType::Multishot].buildTime;
                selectedTower->damageUpgradeLevel = 0;
                selectedTower->speedUpgradeLevel = 0;
                showPopup = false;
              }
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
        int damage =
            stats.damage + (t.damageUpgradeLevel * stats.upgDamageAmount);
        float maxCooldown =
            std::max(0.1f, stats.cooldown -
                               (t.speedUpgradeLevel * stats.upgSpeedAmount));

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
    if ((*it)->hasReachedEnd(pathPoints.size()) || (*it)->isDead()) {
      if ((*it)->isDead()) {
        money += 10;
      }
      it = enemies.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::render() {
  window.clear(sf::Color::Black);
  map.draw(window);

  for (const auto &t : towers) {
    sf::Sprite towerSprite;
    int fw = 70, fh = 130;
    if (t.type == TowerType::Empty) {
      towerSprite.setTexture(towerUnbuiltTex);
      towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
    } else if (t.type == TowerType::Base) {
      towerSprite.setTexture(towerLvl1Tex);
      towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
    } else if (t.type == TowerType::Sniper) {
      towerSprite.setTexture(towerLvl2Tex);
      towerSprite.setTextureRect(sf::IntRect(towerAnimFrame * fw, 0, fw, fh));
    } else if (t.type == TowerType::Multishot) {
      towerSprite.setTexture(towerLvl3Tex);
      towerSprite.setTextureRect(sf::IntRect(towerAnimFrame * fw, 0, fw, fh));
    }
    towerSprite.setOrigin(fw / 2.f, fh / 2.f + 15.f);
    towerSprite.setPosition(t.position);

    // Add visual indicator for building
    if (t.state == TowerState::Building || t.state == TowerState::Upgrading) {
      towerSprite.setColor(
          sf::Color(100, 100, 100, 200)); // Darken while building
    }

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
        numBtns = 4; // 0: Speed, 1: Dmg, 2: Sniper, 3: Multishot
      } else if (selectedTower->type == TowerType::Multishot) {
        numBtns = 3; // 0: Speed, 1: Dmg, 2: Targets
      } else {
        numBtns = 2; // 0: Speed, 1: Damage
      }

      for (int i = 0; i < numBtns; ++i) {
        if (i >= 4)
          continue;

        int cost = 0;
        bool isMax = false;
        if (selectedTower->type == TowerType::Empty && i == 0)
          cost = towerConfigs[TowerType::Base].cost;
        else if (selectedTower->type != TowerType::Empty) {
          TowerStats stats = towerConfigs[selectedTower->type];
          if (i == 0)
            cost = stats.upgSpeedCost;
          else if (i == 1)
            cost = stats.upgDamageCost;
          else if (selectedTower->type == TowerType::Base) {
            if (i == 2)
              cost = towerConfigs[TowerType::Sniper].cost;
            if (i == 3)
              cost = towerConfigs[TowerType::Multishot].cost;
          } else if (selectedTower->type == TowerType::Multishot && i == 2) {
            if (selectedTower->targetsUpgradeLevel < stats.targetUpgrades.size()) {
                cost = stats.targetUpgrades[selectedTower->targetsUpgradeLevel].first;
            } else {
                isMax = true;
            }
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

  window.display();
}
