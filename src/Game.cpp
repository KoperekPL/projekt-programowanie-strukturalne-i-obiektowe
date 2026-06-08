#include "Game.h"
#include <iostream>
#include <cmath>

Game::Game() :
    window(sf::VideoMode(1920, 1080), "Tower Defense", sf::Style::Fullscreen),
    playerSpeed(300.f),
    showPopup(false),
    selectedTower(nullptr),
    levelBtns(4),
    levelTexts(4)
{
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

    for (const auto& spot : map.getTowerSpots()) {
        towers.push_back({spot, 0});
    }

    popupBg.setSize(sf::Vector2f(400.f, 200.f));
    popupBg.setFillColor(sf::Color(50, 50, 50, 240));
    popupBg.setOutlineThickness(2.f);
    popupBg.setOutlineColor(sf::Color::White);
    popupBg.setOrigin(200.f, 100.f);
    popupBg.setPosition(640.f, 360.f);

    closeBtn.setSize(sf::Vector2f(30.f, 30.f));
    closeBtn.setFillColor(sf::Color::Red);
    closeBtn.setOrigin(15.f, 15.f);
    closeBtn.setPosition(815.f, 285.f);

    cross1.setSize(sf::Vector2f(30.f, 4.f));
    cross1.setFillColor(sf::Color::White);
    cross1.setOrigin(15.f, 2.f);
    cross1.setPosition(815.f, 285.f);
    cross1.setRotation(45.f);

    cross2.setSize(sf::Vector2f(30.f, 4.f));
    cross2.setFillColor(sf::Color::White);
    cross2.setOrigin(15.f, 2.f);
    cross2.setPosition(815.f, 285.f);
    cross2.setRotation(-45.f);

    hasFont = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);

    for (int i = 0; i < 4; ++i) {
        levelBtns[i].setSize(sf::Vector2f(40.f, 40.f));
        levelBtns[i].setFillColor(sf::Color(100, 100, 100));
        levelBtns[i].setOutlineThickness(2.f);
        levelBtns[i].setOutlineColor(sf::Color::White);
        levelBtns[i].setPosition(530.f + i * 60.f, 400.f);

        if (hasFont) {
            levelTexts[i].setFont(font);
            levelTexts[i].setString(std::to_string(i));
            levelTexts[i].setCharacterSize(20);
            levelTexts[i].setFillColor(sf::Color::White);
            sf::FloatRect b = levelTexts[i].getLocalBounds();
            levelTexts[i].setOrigin(b.width / 2.f, b.height / 2.f);
            levelTexts[i].setPosition(550.f + i * 60.f, 415.f);
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
    const auto& pathPoints = map.getPathPoints();

    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::P) {
                if (!pathPoints.empty()) {
                    enemies.push_back(std::make_unique<Enemy>(pathPoints[0], &enemyTexD, &enemyTexS, &enemyTexU));
                }
            }

            if (event.key.code == sf::Keyboard::E && !showPopup) {
                float interactionDist = 60.f;
                selectedTower = nullptr;
                for (auto& t : towers) {
                    float dx = player.getPosition().x - t.position.x;
                    float dy = player.getPosition().y - t.position.y;
                    if (std::sqrt(dx*dx + dy*dy) < interactionDist) {
                        showPopup = true;
                        selectedTower = &t;
                        popupText = "MENU WIEZY\nWybierz poziom (0-3)";
                        break;
                    }
                }
                if (!showPopup) {
                    for (const auto& spot : map.getStoreSpots()) {
                        float dx = player.getPosition().x - spot.x;
                        float dy = player.getPosition().y - spot.y;
                        if (std::sqrt(dx*dx + dy*dy) < interactionDist) {
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
                    text.setPosition(640.f, 360.f);
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && showPopup) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                handlePopupClick(worldPos.x, worldPos.y);
            }
        }
    }
}

void Game::handlePopupClick(float mx, float my) {
    if (mx >= 800.f && mx <= 830.f && my >= 270.f && my <= 300.f) {
        showPopup = false;
    }
    if (selectedTower) {
        for (int i = 0; i < 4; ++i) {
            if (mx >= 530.f + i * 60.f && mx <= 570.f + i * 60.f && my >= 400.f && my <= 440.f) {
                selectedTower->level = i;
            }
        }
    }
}

void Game::update(float dt) {
    if (!showPopup) {
        sf::Vector2f playerMove(0.f, 0.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) playerMove.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) playerMove.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) playerMove.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) playerMove.x += 1.f;

        if (playerMove.x != 0.f || playerMove.y != 0.f) {
            float len = std::sqrt(playerMove.x * playerMove.x + playerMove.y * playerMove.y);
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
        if (pos.x < 20.f) pos.x = 20.f;
        if (pos.x > 1260.f) pos.x = 1260.f;
        if (pos.y < 20.f) pos.y = 20.f;
        if (pos.y > 700.f) pos.y = 700.f;
        player.setPosition(pos);
    }

    const auto& pathPoints = map.getPathPoints();
    
    towerAnimTime += dt;
    if (towerAnimTime >= 0.15f) {
        towerAnimTime = 0.f;
        towerAnimFrame = (towerAnimFrame + 1) % 4;
    }

    for (auto it = enemies.begin(); it != enemies.end();) {
        (*it)->update(dt, pathPoints);
        if ((*it)->hasReachedEnd(pathPoints.size())) {
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black);
    map.draw(window);

    for (const auto& t : towers) {
        sf::Sprite towerSprite;
        int fw = 70, fh = 130;
        if (t.level == 0) {
            towerSprite.setTexture(towerUnbuiltTex);
            towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
        } else if (t.level == 1) {
            towerSprite.setTexture(towerLvl1Tex);
            towerSprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
        } else if (t.level == 2) {
            towerSprite.setTexture(towerLvl2Tex);
            towerSprite.setTextureRect(sf::IntRect(towerAnimFrame * fw, 0, fw, fh));
        } else if (t.level == 3) {
            towerSprite.setTexture(towerLvl3Tex);
            towerSprite.setTextureRect(sf::IntRect(towerAnimFrame * fw, 0, fw, fh));
        }
        towerSprite.setOrigin(fw / 2.f, fh / 2.f + 15.f);
        towerSprite.setPosition(t.position);
        window.draw(towerSprite);

        if (t.level > 0) {
            float range = 50.f + t.level * 50.f;
            sf::CircleShape rangeCircle(range);
            rangeCircle.setOrigin(range, range);
            rangeCircle.setPosition(t.position);
            rangeCircle.setFillColor(sf::Color(100, 100, 100, 50));
            rangeCircle.setOutlineThickness(1.f);
            rangeCircle.setOutlineColor(sf::Color(150, 150, 150, 100));
            window.draw(rangeCircle);

            for (const auto& enemy : enemies) {
                sf::Vector2f ePos = enemy->getPosition();
                float dx = ePos.x - t.position.x;
                float dy = ePos.y - t.position.y;
                if (std::sqrt(dx*dx + dy*dy) <= range) {
                    sf::Vertex line[] = {
                        sf::Vertex(t.position, sf::Color::Yellow),
                        sf::Vertex(ePos, sf::Color::Yellow)
                    };
                    window.draw(line, 2, sf::Lines);
                    break;
                }
            }
        }
    }

    for (const auto &enemy : enemies) {
        enemy->draw(window);
    }
    window.draw(player);

    if (showPopup) {
        window.draw(popupBg);
        window.draw(closeBtn);
        window.draw(cross1);
        window.draw(cross2);
        if (hasFont) window.draw(text);
        if (selectedTower) {
            for (int i = 0; i < 4; ++i) {
                if (selectedTower->level == i) {
                    levelBtns[i].setFillColor(sf::Color(150, 150, 150));
                } else {
                    levelBtns[i].setFillColor(sf::Color(100, 100, 100));
                }
                window.draw(levelBtns[i]);
                if (hasFont) window.draw(levelTexts[i]);
            }
        }
    }

    window.display();
}
