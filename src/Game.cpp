#include "Game.h"
#include "Config.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>


#include "WaveManager.h"
#include "SaveManager.h"

Game::Game(const std::string& name, const std::string& mapPath, bool isNewGame)
    : currentSaveName(name),
      window(sf::VideoMode(1280, 720), "Tower Defense C++"),
      currentMapPath(mapPath),
      isFullscreen(false), playerSpeed(200.f), showPopup(false),
      selectedTower(nullptr), levelBtns(5), levelTexts(5), money(100),
      debugMode(false), baseHp(100), maxBaseHp(100), playerHp(50), maxPlayerHp(50) {
  window.setFramerateLimit(60);
  window.setView(sf::View(sf::FloatRect(0.f, 0.f, 1280.f, 720.f)));

  if (!isNewGame) {
      SaveManager::loadGame(currentSaveName, gameObjects, playerHp, maxPlayerHp, baseHp, maxBaseHp, money, timeScale, waveManager, currentMapPath);
  }

  if (!map.loadFromFile(currentMapPath)) {
    std::cerr << "Failed to load " << currentMapPath << "\n";
  }

  assets.loadAllTextures();
  assets.loadAudio();

  int randomMusicNum = (rand() % 9) + 1;
  std::string musicFile = "../../../assets/sound/music" + std::to_string(randomMusicNum) + ".mp3";
  if (bgMusic.openFromFile(musicFile)) {
      bgMusic.setVolume(musicVolume);
      bgMusic.setLoop(true);
      bgMusic.play();
  } else {
      std::cerr << "Failed to load bg music: " << musicFile << "\n";
  }

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

  if (isNewGame) {
      for (const auto &spot : map.getTowerSpots()) {
        auto tower = std::make_shared<Tower>(spot);
        gameObjects.push_back(tower);
      }
      for (const auto &spot : map.getHealerSpots()) {
        auto tower = std::make_shared<Tower>(spot);
        tower->isHealerSpot = true;
        gameObjects.push_back(tower);
      }
  } else {
      for (const auto& spot : map.getHealerSpots()) {
          for (auto& obj : gameObjects) {
              if (auto t = dynamic_cast<Tower*>(obj.get())) {
                  float dx = t->position.x - spot.x;
                  float dy = t->position.y - spot.y;
                  if (std::sqrt(dx*dx + dy*dy) < 5.f) {
                      t->isHealerSpot = true;
                  }
              }
          }
      }
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
    if (currentState == GameState::Playing) {
        processEvents();
        update(dt);
        render();
        window.display();
    } else if (currentState == GameState::Paused) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            processPauseEvents(event);
        }
        renderPauseMenu();
        window.display();
    } else if (currentState == GameState::GameOver) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close();
            }
        }
        
        window.clear(sf::Color::Black);
        if (hasFont) {
            sf::Text overText("KONIEC GRY!", font, 60);
            overText.setFillColor(sf::Color::Red);
            sf::FloatRect bounds = overText.getLocalBounds();
            overText.setPosition(1280.f/2.f - bounds.width/2.f, 300.f);
            
            sf::Text subText("Nacisnij ESC aby wyjsc", font, 30);
            subText.setFillColor(sf::Color::White);
            sf::FloatRect subBounds = subText.getLocalBounds();
            subText.setPosition(1280.f/2.f - subBounds.width/2.f, 400.f);
            
            window.draw(overText);
            window.draw(subText);
        }
        window.display();
    }
  }
}

void Game::processEvents() {
    sf::Event event;

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

      if (event.key.code == sf::Keyboard::Escape) {
          currentState = GameState::Paused;
          saveFeedbackTimer = 0.f;
      }

      if (event.key.code == sf::Keyboard::Z && event.key.control &&
          event.key.shift) {
        debugMode = !debugMode;
      }

      if (debugMode) {
          if (event.key.code == sf::Keyboard::P) {
              if (!map.getPathA().empty() && !enemyConfigs.empty()) {
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
                  const auto& chosenPath = map.getRandomPath();
                  auto enemy = std::make_shared<Enemy>(chosenPath[0], stats, tD, tS, tU, &chosenPath);
                  gameObjects.push_back(enemy);
              }
          }

        if (event.key.code == sf::Keyboard::O) {
          for (auto &obj : gameObjects) {
            if (auto enemy = dynamic_cast<Enemy*>(obj.get())) {
              enemy->takeDamage(5);
            }
          }
        }

        if (event.key.code == sf::Keyboard::K) {
          money += 1000;
        }
      }

      if (event.key.code == sf::Keyboard::Space) {
          if (meleeCooldownTimer <= 0.f) {
              float range = hasSword ? 80.f : 50.f;
              int damage = hasSword ? 30 : 10;
              bool hitAny = false;
              
              for (auto& obj : gameObjects) {
                  if (auto enemy = dynamic_cast<Enemy*>(obj.get())) {
                      if (enemy->isDead()) continue;
                      float dx = enemy->getPosition().x - player.getPosition().x;
                      float dy = enemy->getPosition().y - player.getPosition().y;
                      float dist = std::sqrt(dx*dx + dy*dy);
                      if (dist <= range) {
                          enemy->takeDamage(damage);
                          hitAny = true;
                          AttackSlash slash;
                          slash.position = player.getPosition();
                          slash.angle = atan2(dy, dx) * 180.f / 3.14159f;
                          slash.lifeTime = 0.2f;
                          attackSlashes.push_back(slash);
                          break; // Only hit nearest one enemy for now
                      }
                  }
              }
              if (hitAny) meleeCooldownTimer = 1.0f;
          }
      }

      if (event.key.code == sf::Keyboard::E && !showPopup) {
        float interactionDist = 60.f;
        selectedTower = nullptr;
        for (auto &obj : gameObjects) {
          if (auto t = dynamic_cast<Tower*>(obj.get())) {
            float dx = player.getPosition().x - t->position.x;
            float dy = player.getPosition().y - t->position.y;
            if (std::sqrt(dx * dx + dy * dy) < interactionDist) {
              showPopup = true;
              selectedTower = t;

              if (t->state == TowerState::Building ||
                  t->state == TowerState::Upgrading) {
                popupText = "WIEZA W BUDOWIE...";
              } else {
                if (t->type == TowerType::Empty) {
                  if (t->isHealerSpot) {
                      popupText = "BUDUJ WIEZE\n0 - Leczaca (" +
                                  std::to_string(towerConfigs[TowerType::Healer].cost) + "$)";
                  } else {
                      popupText = "BUDUJ WIEZE\n0 - Podstawowa (" +
                                  std::to_string(towerConfigs[TowerType::Base].cost) + "$)";
                  }
                } else {
                  TowerStats stats = towerConfigs[t->type];
                  int currDmg = stats.damage;
                  for (int i = 0; i < t->damageUpgradeLevel && i < static_cast<int>(stats.damageUpgrades.size()); ++i)
                      currDmg += stats.damageUpgrades[i].second;
                  
                  float currSpd = stats.cooldown;
                  for (int i = 0; i < t->speedUpgradeLevel && i < static_cast<int>(stats.speedUpgrades.size()); ++i)
                      currSpd -= stats.speedUpgrades[i].second;
                  currSpd = std::max(0.1f, currSpd);

                std::string typeName =
                    (t->type == TowerType::Base)
                        ? "PODSTAWOWA"
                        : ((t->type == TowerType::Sniper) ? "SNAJPER"
                                                         : ((t->type == TowerType::Healer) ? "HEALER" : "MULTISHOT"));

                char buf[64];
                snprintf(buf, sizeof(buf), "%.2fs", currSpd);
                
                std::string extraStats = "";
                if (t->type == TowerType::Multishot) {
                    int maxTgt = stats.baseTargets;
                    if (t->targetsUpgradeLevel > 0 && t->targetsUpgradeLevel <= static_cast<int>(stats.targetUpgrades.size())) {
                        maxTgt = stats.targetUpgrades[t->targetsUpgradeLevel - 1].second;
                    }
                    extraStats = " | Cele: " + std::to_string(maxTgt);
                }

                std::string speedUpgText = "MAX";
                if (t->speedUpgradeLevel < static_cast<int>(stats.speedUpgrades.size())) {
                    auto upg = stats.speedUpgrades[t->speedUpgradeLevel];
                    char buf2[64];
                    snprintf(buf2, sizeof(buf2), "%.2f", upg.second);
                    speedUpgText = "-" + std::string(buf2) + "s za " + std::to_string(upg.first) + "$";
                }

                std::string dmgUpgText = "MAX";
                if (t->damageUpgradeLevel < static_cast<int>(stats.damageUpgrades.size())) {
                    auto upg = stats.damageUpgrades[t->damageUpgradeLevel];
                    dmgUpgText = "+" + std::to_string(upg.second) + " za " + std::to_string(upg.first) + "$";
                }

                popupText =
                    "WIEZA " + typeName + (t->hasTimedUpgrade ? " (BOOST)" : "") + "\nObr: " + std::to_string(currDmg) +
                    " | Przelad: " + std::string(buf) + extraStats + "\n" +
                    "0 - Szybkosc (" + speedUpgText + ")\n" +
                    "1 - Obrazenia (" + dmgUpgText + ")";

                int nextBtnIndex = 2;
                if (t->type == TowerType::Base) {
                  bool isMaxUpgraded = (t->speedUpgradeLevel >= static_cast<int>(stats.speedUpgrades.size())) &&
                                       (t->damageUpgradeLevel >= static_cast<int>(stats.damageUpgrades.size()));
                  if (isMaxUpgraded) {
                    popupText += "\n2 - Ewolucja Snajper (" + std::to_string(towerConfigs[TowerType::Sniper].cost) + "$)\n" + 
                                 "3 - Ewolucja Multishot (" + std::to_string(towerConfigs[TowerType::Multishot].cost) + "$)";
                  } else {
                    popupText += "\n2 - Ewolucja Snajper (Wymagany MAX)\n" 
                                 "3 - Ewolucja Multishot (Wymagany MAX)";
                  }
                  nextBtnIndex = 4;
                } else if (t->type == TowerType::Multishot) {
                    if (t->targetsUpgradeLevel < static_cast<int>(stats.targetUpgrades.size())) {
                        int cost = stats.targetUpgrades[t->targetsUpgradeLevel].first;
                        int nextTgt = stats.targetUpgrades[t->targetsUpgradeLevel].second;
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

    if (event.type == sf::Event::MouseButtonPressed) {
      if (event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f worldPos = window.mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

        if (!showPopup && !showShop && worldPos.x >= 1100.f && worldPos.x <= 1250.f && worldPos.y >= 10.f && worldPos.y <= 50.f) {
            showShop = true;
            return;
        }

        if (showShop) {
            if (worldPos.x >= 850.f && worldPos.x <= 880.f && worldPos.y >= 195.f && worldPos.y <= 225.f) {
                showShop = false;
            }
            if (worldPos.x >= 400.f && worldPos.x <= 600.f && worldPos.y >= 300.f && worldPos.y <= 360.f) {
                if (!hasSword && money >= 500) {
                    money -= 500;
                    hasSword = true;
                }
            }
            if (worldPos.x >= 650.f && worldPos.x <= 850.f && worldPos.y >= 300.f && worldPos.y <= 360.f) {
                if (money >= 1000 && baseHp < maxBaseHp) {
                    money -= 1000;
                    baseHp += 20;
                    if (baseHp > maxBaseHp) baseHp = maxBaseHp;
                }
            }
        } else if (showPopup) {
            handlePopupClick(worldPos.x, worldPos.y);
        }
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
        if (selectedTower->type == TowerType::Empty) {
          if (i == 0) {
              if (selectedTower->isHealerSpot) {
                  int cost = towerConfigs[TowerType::Healer].cost;
                  if (money >= cost) {
                    money -= cost;
                    selectedTower->type = TowerType::Healer;
                    selectedTower->state = TowerState::Building;
                    selectedTower->buildTimer = towerConfigs[TowerType::Healer].buildTime;
                    selectedTower->buildSound.setBuffer(assets.buildBuf);
                    selectedTower->buildSound.setVolume(sfxVolume);
                    selectedTower->buildSound.setLoop(true);
                    selectedTower->buildSound.play();
                    showPopup = false;
                  }
              } else {
                  int cost = towerConfigs[TowerType::Base].cost;
                  if (money >= cost) {
                    money -= cost;
                    selectedTower->type = TowerType::Base;
                    selectedTower->state = TowerState::Building;
                    selectedTower->buildTimer = towerConfigs[TowerType::Base].buildTime;
                    selectedTower->buildSound.setBuffer(assets.buildBuf);
                    selectedTower->buildSound.setVolume(sfxVolume);
                    selectedTower->buildSound.setLoop(true);
                    selectedTower->buildSound.play();
                    showPopup = false;
                  }
              }
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
                  selectedTower->buildSound.setBuffer(assets.buildBuf);
                  selectedTower->buildSound.setVolume(sfxVolume);
                  selectedTower->buildSound.setLoop(true);
                  selectedTower->buildSound.play();
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
                  selectedTower->buildSound.setBuffer(assets.buildBuf);
                  selectedTower->buildSound.setVolume(sfxVolume);
                  selectedTower->buildSound.setLoop(true);
                  selectedTower->buildSound.play();
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
  float realDt = dt;
  dt *= timeScale;

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

  waveManager.update(dt, gameObjects, enemyConfigs, map, assets);
  
  autoSaveTimer += realDt;
  if (autoSaveTimer >= 300.f) {
      autoSaveTimer = 0.f;
      SaveManager::saveGame(currentSaveName, gameObjects, playerHp, maxPlayerHp, baseHp, maxBaseHp, money, timeScale, waveManager, currentMapPath);
      saveFeedbackTimer = 2.0f; // Pokaż powiadomienie przez 2 sekundy (nawet z auto-zapisu)
  }

  if (saveFeedbackTimer > 0.f) {
      saveFeedbackTimer -= realDt;
  }

  if (playerHp <= 0 || baseHp <= 0) {
      currentState = GameState::GameOver;
  }

  towerAnimTime += dt;
  if (towerAnimTime >= 0.15f) {
    towerAnimTime = 0.f;
    towerAnimFrame = (towerAnimFrame + 1) % 4;
  }

  for (auto it = attackSlashes.begin(); it != attackSlashes.end(); ) {
      it->lifeTime -= dt;
      if (it->lifeTime <= 0.f) it = attackSlashes.erase(it);
      else ++it;
  }

  if (meleeCooldownTimer > 0.f) meleeCooldownTimer -= dt;

  for (auto &obj : gameObjects) {
    if (auto t = dynamic_cast<Tower*>(obj.get())) {
      t->update(dt);

    if (t->state == TowerState::Idle && t->type != TowerType::Empty) {
      if (t->cooldownTimer <= 0.f && t->type == TowerType::Healer) {
          float dx = t->position.x - player.getPosition().x;
          float dy = t->position.y - player.getPosition().y;
          if (std::sqrt(dx*dx + dy*dy) < towerConfigs[TowerType::Healer].range) {
              playerHp += 5;
              if (playerHp > maxPlayerHp) playerHp = maxPlayerHp;
              t->cooldownTimer = towerConfigs[TowerType::Healer].cooldown;
          }
      }
      if (t->cooldownTimer <= 0.f && t->type != TowerType::Healer) {
        TowerStats stats = towerConfigs[t->type];
        float range = stats.range;
        
        int damage = stats.damage;
        for (int i = 0; i < t->damageUpgradeLevel && i < static_cast<int>(stats.damageUpgrades.size()); ++i) {
            damage += stats.damageUpgrades[i].second;
        }

        float cooldown = stats.cooldown;
        for (int i = 0; i < t->speedUpgradeLevel && i < static_cast<int>(stats.speedUpgrades.size()); ++i) {
            cooldown -= stats.speedUpgrades[i].second;
        }
        float maxCooldown = std::max(0.1f, cooldown);

        if (t->hasTimedUpgrade) {
            damage = static_cast<int>(damage * stats.timedUpgrade.damageMultiplier);
            maxCooldown *= stats.timedUpgrade.cooldownMultiplier;
            maxCooldown = std::max(0.01f, maxCooldown);
        }

        if (t->type == TowerType::Base || t->type == TowerType::Sniper) {
          for (auto &obj2 : gameObjects) {
            if (auto enemy = dynamic_cast<Enemy*>(obj2.get())) {
              if (enemy->isDead())
                continue;
              sf::Vector2f ePos = enemy->getPosition();
              float dx = ePos.x - t->position.x;
              float dy = ePos.y - t->position.y;
              if (std::sqrt(dx * dx + dy * dy) <= range) {
                enemy->takeDamage(damage);
                t->cooldownTimer = maxCooldown;
                
                t->targetRotation = atan2(dy, dx) * 180.f / 3.14159f;
                
                Laser laser;
                laser.start = t->position;
                laser.end = ePos;
                laser.color = sf::Color(255, 0, 0, 200);
                laser.lifeTime = 0.2f;
                activeLasers.push_back(laser);
                
                sf::Sound& s = t->buildSound;
                s.setBuffer(t->type == TowerType::Sniper ? assets.sniperBuf : assets.gunshotBuf);
                s.setVolume(t->type == TowerType::Sniper ? sfxVolume * 0.3f : sfxVolume);
                s.setLoop(false);
                s.play();

                break;
              }
            }
          }
        } else if (t->type == TowerType::Multishot) {
          int maxTargets = stats.baseTargets;
          if (t->targetsUpgradeLevel > 0 && t->targetsUpgradeLevel <= stats.targetUpgrades.size()) {
              maxTargets = stats.targetUpgrades[t->targetsUpgradeLevel - 1].second;
          }
          
          std::vector<std::pair<float, Enemy*>> targetsInRange;
          for (auto &obj2 : gameObjects) {
            if (auto enemy = dynamic_cast<Enemy*>(obj2.get())) {
              if (enemy->isDead())
                continue;
              sf::Vector2f ePos = enemy->getPosition();
              float dx = ePos.x - t->position.x;
              float dy = ePos.y - t->position.y;
              float dist = std::sqrt(dx * dx + dy * dy);
              if (dist <= range) {
                targetsInRange.push_back({dist, enemy});
              }
            }
          }
          
          if (!targetsInRange.empty()) {
              std::sort(targetsInRange.begin(), targetsInRange.end());
              int targetsHit = 0;
              for (auto &pair : targetsInRange) {
                  if (targetsHit >= maxTargets) break;
                  pair.second->takeDamage(damage);
                  targetsHit++;
                  
                  Laser laser;
                  laser.start = t->position;
                  laser.end = pair.second->getPosition();
                  laser.color = sf::Color(0, 0, 255, 200);
                  laser.lifeTime = 0.2f;
                  activeLasers.push_back(laser);
              }
              t->cooldownTimer = maxCooldown;
              
              sf::Sound& s = t->buildSound;
              s.setBuffer(assets.gunshotBuf);
              s.setVolume(sfxVolume);
              s.setLoop(false);
              s.play();
          }
        }
      }
    }
    }
  }

  for (auto it = gameObjects.begin(); it != gameObjects.end(); ) {
      if (auto enemy = dynamic_cast<Enemy*>(it->get())) {
          enemy->update(dt);
          
          sf::Vector2f ePos = enemy->getPosition();
          sf::Vector2f pPos = player.getPosition();
          float dx = ePos.x - pPos.x;
          float dy = ePos.y - pPos.y;
          float dist = std::sqrt(dx * dx + dy * dy);
          if (dist < 40.f && !enemy->isDead()) {
              if (enemy->slowTimer <= 0.f) {
                  playerHp -= enemy->getPlayerDamage();
                  if (playerHp < 0) playerHp = 0;
              }
              enemy->slowTimer = 0.5f;
          }

          if (enemy->hasReachedEnd() || enemy->isDead()) {
            if (enemy->hasReachedEnd()) {
              baseHp -= enemy->getCastleDamage();
              if (baseHp < 0) baseHp = 0;
            } else {
              money += 10;
            }
            it = gameObjects.erase(it);
          } else {
            ++it;
          }
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
      for (const auto &obj : gameObjects) {
          if (auto t = dynamic_cast<Tower*>(obj.get())) {
              if (t->type != TowerType::Empty) {
                  float dx = worldPos.x - t->position.x;
                  float dy = worldPos.y - t->position.y;
                  if (std::sqrt(dx * dx + dy * dy) < 35.f) {
                      hoveredTower = t;
                      break;
                  }
              }
          }
      }
  }

  map.draw(window);

  for (const auto &obj : gameObjects) {
    if (auto t = dynamic_cast<Tower*>(obj.get())) {
    sf::Sprite towerSprite;
    int fw = 70, fh = 130;
    
    if (t->type == TowerType::Empty) {
      towerSprite.setTexture(assets.towerUpgradeTexs[0]);
      towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
    } else {
      sf::Texture* activeTex = nullptr;
      int frameIndex = 0;

      if (t->type == TowerType::Base) {
          activeTex = &assets.towerUpgradeTexs[3]; // 4upgrade.png
          bool isMax = (t->speedUpgradeLevel >= 3 && t->damageUpgradeLevel >= 3);
          bool isLvl2 = (!isMax && t->speedUpgradeLevel + t->damageUpgradeLevel >= 4);
          bool isLvl1 = (!isMax && !isLvl2 && t->speedUpgradeLevel + t->damageUpgradeLevel >= 2);
          if (isMax) frameIndex = 3;
          else if (isLvl2) frameIndex = 2;
          else if (isLvl1) frameIndex = 1;
          else frameIndex = 0;
      } else if (t->type == TowerType::Sniper) {
          bool isUpgraded = (t->speedUpgradeLevel > 0 || t->damageUpgradeLevel > 0);
          if (isUpgraded) {
              activeTex = &assets.towerUpgradeTexs[6]; // 7upgrade.png
              frameIndex = 3; // Ostatnia forma z 7upgrade
          } else {
              activeTex = &assets.towerUpgradeTexs[4]; // 5upgrade.png
              frameIndex = 0; 
          }
      } else if (t->type == TowerType::Multishot) {
          activeTex = &assets.towerUpgradeTexs[5]; // 6upgrade.png
          bool isUpgraded = (t->speedUpgradeLevel > 0 || t->damageUpgradeLevel > 0 || t->targetsUpgradeLevel > 0);
          if (isUpgraded) frameIndex = 3; // Ostatnia tekstura z pliku 6upgrade.png
          else frameIndex = 0; // Pierwsza tekstura
      } else if (t->type == TowerType::Healer) {
          activeTex = &assets.towerUpgradeTexs[4]; // Use Sniper texture for now as requested
          frameIndex = 0;
      }

      towerSprite.setTexture(*activeTex);
      towerSprite.setTextureRect(sf::IntRect(frameIndex * fw, 0, fw, fh));
    }
    
    towerSprite.setOrigin(fw / 2.f, fh / 2.f + 15.f);
    towerSprite.setPosition(t->position);

    window.draw(towerSprite);

    // Draw building progress bar
    if (t->state == TowerState::Building || t->state == TowerState::Upgrading) {
      float maxTimer = 1.f;
      if (t->type == TowerType::Base)
        maxTimer = towerConfigs[TowerType::Base].buildTime;
      else if (t->type == TowerType::Sniper)
        maxTimer = towerConfigs[TowerType::Sniper].buildTime;
      else if (t->type == TowerType::Multishot)
        maxTimer = towerConfigs[TowerType::Multishot].buildTime;
      else if (t->type == TowerType::Healer)
        maxTimer = towerConfigs[TowerType::Healer].buildTime;

      float progress = 1.f - (t->buildTimer / maxTimer);
      if (progress < 0.f)
        progress = 0.f;

      sf::RectangleShape bgBar(sf::Vector2f(40.f, 5.f));
      bgBar.setFillColor(sf::Color::Red);
      bgBar.setOrigin(20.f, 2.5f);
      bgBar.setPosition(t->position.x, t->position.y - 40.f);

      sf::RectangleShape fgBar(sf::Vector2f(40.f * progress, 5.f));
      fgBar.setFillColor(sf::Color::Blue);
      fgBar.setOrigin(20.f, 2.5f);
      fgBar.setPosition(t->position.x, t->position.y - 40.f);

      window.draw(bgBar);
      window.draw(fgBar);
    }

    if ((t->type == TowerType::Healer || (t->type == TowerType::Empty && t->isHealerSpot)) && hasFont) {
        sf::Text hText("H", font, 24);
        hText.setFillColor(sf::Color::Green);
        hText.setOutlineColor(sf::Color::Black);
        hText.setOutlineThickness(2.f);
        sf::FloatRect bounds = hText.getLocalBounds();
        hText.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        hText.setPosition(t->position.x, t->position.y - 15.f);
        window.draw(hText);
    }

    if (t->type != TowerType::Empty && t->state == TowerState::Idle) {
      // Draw damage upgrade indicators (red squares/dots)
      for (int i = 0; i < t->damageUpgradeLevel; ++i) {
        sf::RectangleShape dmgDot(sf::Vector2f(6.f, 6.f));
        dmgDot.setFillColor(sf::Color::Red);
        dmgDot.setOutlineThickness(1.f);
        dmgDot.setOutlineColor(sf::Color::Black);
        dmgDot.setOrigin(3.f, 3.f);
        dmgDot.setPosition(t->position.x - 20.f + i * 8.f, t->position.y + 30.f);
        window.draw(dmgDot);
      }
      
      // Draw speed upgrade indicators (cyan squares/dots)
      for (int i = 0; i < t->speedUpgradeLevel; ++i) {
        sf::RectangleShape spdDot(sf::Vector2f(6.f, 6.f));
        spdDot.setFillColor(sf::Color::Cyan);
        spdDot.setOutlineThickness(1.f);
        spdDot.setOutlineColor(sf::Color::Black);
        spdDot.setOrigin(3.f, 3.f);
        spdDot.setPosition(t->position.x + 5.f + i * 8.f, t->position.y + 30.f);
        window.draw(spdDot);
      }
      
      // Draw timed upgrade bar (yellow progress bar)
      if (t->hasTimedUpgrade) {
        float maxTimer = towerConfigs[t->type].timedUpgrade.duration;
        float progress = t->timedUpgradeTimer / maxTimer;
        if (progress < 0.f) progress = 0.f;

        sf::RectangleShape bgBar(sf::Vector2f(40.f, 4.f));
        bgBar.setFillColor(sf::Color(50, 50, 50));
        bgBar.setOutlineThickness(1.f);
        bgBar.setOutlineColor(sf::Color::Black);
        bgBar.setOrigin(20.f, 2.f);
        bgBar.setPosition(t->position.x, t->position.y - 55.f);

        sf::RectangleShape fgBar(sf::Vector2f(40.f * progress, 4.f));
        fgBar.setFillColor(sf::Color::Yellow);
        fgBar.setOrigin(20.f, 2.f);
        fgBar.setPosition(t->position.x, t->position.y - 55.f);

        window.draw(bgBar);
        window.draw(fgBar);
      }
    }

    if (t == hoveredTower && t->type != TowerType::Empty && t->state == TowerState::Idle && t->type != TowerType::Healer) {
      float range = towerConfigs[t->type].range;
      sf::CircleShape rangeCircle(range);
      rangeCircle.setOrigin(range, range);
      rangeCircle.setPosition(t->position);
      rangeCircle.setFillColor(sf::Color(100, 100, 100, 50));
      rangeCircle.setOutlineThickness(1.f);
      rangeCircle.setOutlineColor(sf::Color(150, 150, 150, 100));
      window.draw(rangeCircle);
    }
    }
  }

  for (auto it = activeLasers.begin(); it != activeLasers.end(); ) {
      it->lifeTime -= 1.0f / 60.f; // roughly 60fps
      if (it->lifeTime <= 0.f) {
          it = activeLasers.erase(it);
      } else {
          float thick = 3.0f;
          sf::Vector2f dir = it->end - it->start;
          float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
          if (len > 0) dir /= len;
          sf::Vector2f norm(-dir.y, dir.x);
          
          int alpha = static_cast<int>(255 * (it->lifeTime / 0.2f));
          if (alpha < 0) alpha = 0;
          if (alpha > 255) alpha = 255;
          sf::Color c = it->color;
          c.a = alpha;

          sf::VertexArray line(sf::Quads, 4);
          line[0].position = it->start + norm * thick;
          line[1].position = it->end + norm * thick;
          line[2].position = it->end - norm * thick;
          line[3].position = it->start - norm * thick;
          for (int i=0; i<4; ++i) line[i].color = c;
          
          window.draw(line);
          ++it;
      }
  }

  for (auto& slash : attackSlashes) {
      sf::RectangleShape rect(sf::Vector2f(60.f, 10.f));
      rect.setOrigin(30.f, 5.f);
      rect.setPosition(slash.position);
      rect.setRotation(slash.angle);
      rect.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(255 * (slash.lifeTime / 0.2f))));
      window.draw(rect);
  }

  for (const auto &obj : gameObjects) {
      if (dynamic_cast<Enemy*>(obj.get())) {
          obj->draw(window);
      }
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
    moneyText.setString("Pieniadze: " + std::to_string(money) + " | Fala: " + std::to_string(waveManager.currentWave) + "/" + std::to_string(waveManager.maxWaves));
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
      } else if (selectedTower->type == TowerType::Healer) {
        numBtns = 0; // Healer has no upgrades
      } else {
        numBtns = 3; // 0: Speed, 1: Damage, 2: Timed
      }

      for (int i = 0; i < numBtns; ++i) {
        if (i >= 5)
          continue;

        int cost = 0;
        bool isMax = false;
        if (selectedTower->type == TowerType::Empty && i == 0) {
          cost = selectedTower->isHealerSpot ? towerConfigs[TowerType::Healer].cost : towerConfigs[TowerType::Base].cost;
        } else if (selectedTower->type != TowerType::Empty) {
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
}

void Game::renderPauseMenu() {
    render();
    
    sf::RectangleShape overlay(sf::Vector2f(1280.f, 720.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    sf::Text pText("PAUZA", font, 60);
    sf::FloatRect pb = pText.getLocalBounds();
    pText.setOrigin(pb.width / 2.f, pb.height / 2.f);
    pText.setPosition(640.f, 150.f);
    window.draw(pText);

    std::string btns[] = {"Kontynuuj", "Zapisz gre", "Wyjdz z gry"};
    for (int i=0; i<3; ++i) {
        sf::RectangleShape btn(sf::Vector2f(300.f, 60.f));
        btn.setOrigin(150.f, 30.f);
        btn.setPosition(640.f, 300.f + i * 80.f);
        btn.setFillColor(sf::Color(100, 100, 100));
        btn.setOutlineThickness(2.f);
        btn.setOutlineColor(sf::Color::White);
        window.draw(btn);
        
        std::string btnLabel = btns[i];
        if (i == 1 && saveFeedbackTimer > 0.f) {
            btnLabel = "ZAPISANO!";
        }

        sf::Text bText(btnLabel, font, 24);
        sf::FloatRect bb = bText.getLocalBounds();
        bText.setOrigin(bb.width / 2.f, bb.height / 2.f);
        bText.setPosition(640.f, 300.f + i * 80.f);
        window.draw(bText);
    }

    sf::Text sfxText("SFX: " + std::to_string((int)sfxVolume) + "%", font, 24);
    sf::FloatRect sfxb = sfxText.getLocalBounds();
    sfxText.setOrigin(sfxb.width / 2.f, sfxb.height / 2.f);
    sfxText.setPosition(450.f, 560.f);
    window.draw(sfxText);

    sf::RectangleShape sfxTrack(sf::Vector2f(200.f, 10.f));
    sfxTrack.setPosition(350.f, 600.f);
    sfxTrack.setFillColor(sf::Color(100, 100, 100));
    window.draw(sfxTrack);

    sf::RectangleShape sfxKnob(sf::Vector2f(10.f, 30.f));
    sfxKnob.setOrigin(5.f, 15.f);
    sfxKnob.setPosition(350.f + (sfxVolume / 100.f) * 200.f, 605.f);
    sfxKnob.setFillColor(sf::Color::White);
    window.draw(sfxKnob);

    sf::Text musText("Muzyka: " + std::to_string((int)musicVolume) + "%", font, 24);
    sf::FloatRect musb = musText.getLocalBounds();
    musText.setOrigin(musb.width / 2.f, musb.height / 2.f);
    musText.setPosition(830.f, 560.f);
    window.draw(musText);

    sf::RectangleShape musTrack(sf::Vector2f(200.f, 10.f));
    musTrack.setPosition(730.f, 600.f);
    musTrack.setFillColor(sf::Color(100, 100, 100));
    window.draw(musTrack);

    sf::RectangleShape musKnob(sf::Vector2f(10.f, 30.f));
    musKnob.setOrigin(5.f, 15.f);
    musKnob.setPosition(730.f + (musicVolume / 100.f) * 200.f, 605.f);
    musKnob.setFillColor(sf::Color::White);
    window.draw(musKnob);

    if (debugMode) {
        sf::Text tsText("Predkosc (Debug): x" + std::to_string(timeScale).substr(0, 4), font, 24);
        sf::FloatRect tsb = tsText.getLocalBounds();
        tsText.setOrigin(tsb.width / 2.f, tsb.height / 2.f);
        tsText.setPosition(640.f, 650.f);
        window.draw(tsText);

        sf::RectangleShape tsTrack(sf::Vector2f(200.f, 10.f));
        tsTrack.setPosition(540.f, 680.f);
        tsTrack.setFillColor(sf::Color(100, 100, 100));
        window.draw(tsTrack);

        float tsProgress = (timeScale - 0.5f) / 19.5f;
        if (tsProgress < 0.f) tsProgress = 0.f;
        if (tsProgress > 1.f) tsProgress = 1.f;

        sf::RectangleShape tsKnob(sf::Vector2f(10.f, 30.f));
        tsKnob.setOrigin(5.f, 15.f);
        tsKnob.setPosition(540.f + tsProgress * 200.f, 685.f);
        tsKnob.setFillColor(sf::Color::Yellow);
        window.draw(tsKnob);
    }
}

void Game::processPauseEvents(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
        currentState = GameState::Playing;
    }
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        float mx = worldPos.x;
        float my = worldPos.y;
        
        for (int i=0; i<3; ++i) {
            if (mx >= 640.f - 150.f && mx <= 640.f + 150.f && my >= 300.f + i * 80.f - 30.f && my <= 300.f + i * 80.f + 30.f) {
                if (i == 0) {
                    currentState = GameState::Playing;
                } else if (i == 1) {
                    SaveManager::saveGame(currentSaveName, gameObjects, playerHp, maxPlayerHp, baseHp, maxBaseHp, money, timeScale, waveManager, currentMapPath);
                    saveFeedbackTimer = 2.0f;
                } else if (i == 2) {
                    window.close();
                }
            }
        }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        float mx = worldPos.x;
        float my = worldPos.y;

        // SFX slider: Track starts at 350, length 200. Y around 605
        if (mx >= 330.f && mx <= 570.f && my >= 580.f && my <= 630.f) {
            sfxVolume = (mx - 350.f) / 200.f * 100.f;
            if (sfxVolume < 0.f) sfxVolume = 0.f;
            if (sfxVolume > 100.f) sfxVolume = 100.f;
        }

        // Music slider: Track starts at 730, length 200. Y around 605
        if (mx >= 710.f && mx <= 950.f && my >= 580.f && my <= 630.f) {
            musicVolume = (mx - 730.f) / 200.f * 100.f;
            if (musicVolume < 0.f) musicVolume = 0.f;
            if (musicVolume > 100.f) musicVolume = 100.f;
            bgMusic.setVolume(musicVolume);
        }

        if (debugMode) {
            if (mx >= 520.f && mx <= 760.f && my >= 660.f && my <= 710.f) {
                float progress = (mx - 540.f) / 200.f;
                if (progress < 0.f) progress = 0.f;
                if (progress > 1.f) progress = 1.f;
                timeScale = 0.5f + progress * 19.5f;
            }
        }
    }
}
