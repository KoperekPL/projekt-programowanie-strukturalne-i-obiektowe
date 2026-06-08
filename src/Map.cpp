#include "Map.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <utility>

Map::Map(float tSize) : tileSize(tSize) {}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc mapy: " << filename << std::endl;
        return false;
    }

    std::vector<std::string> grid;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            grid.push_back(line);
        }
    }
    file.close();

    if (grid.empty()) return false;

    int rows = grid.size();
    int cols = grid[0].size();

    sf::Vector2i startPos(-1, -1);
    
    towerSpots.clear();
    bridgeSpots.clear();
    storeSpots.clear();
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < static_cast<int>(grid[y].size()); ++x) {
            char cell = grid[y][x];
            if (cell == 'S') {
                startPos = {x, y};
            } else if (cell == 'T') {
                towerSpots.push_back({x * tileSize + tileSize / 2.f, y * tileSize + tileSize / 2.f});
            } else if (cell == 'M') {
                bridgeSpots.push_back({x * tileSize + tileSize / 2.f, y * tileSize + tileSize / 2.f});
            } else if (cell == 'K') {
                storeSpots.push_back({x * tileSize + tileSize / 2.f, y * tileSize + tileSize / 2.f});
            }
        }
    }

    pathPoints.clear();
    if (startPos.x == -1) {
        std::cerr << "Brak punktu startowego 'S' na mapie!" << std::endl;
        return false;
    }

    sf::Vector2i current = startPos;
    std::set<std::pair<int, int>> visited;
    
    while (true) {
        visited.insert({current.x, current.y});
        pathPoints.push_back({current.x * tileSize + tileSize / 2.f, current.y * tileSize + tileSize / 2.f});

        if (grid[current.y][current.x] == 'E') {
            break;
        }

        sf::Vector2i nextStep(-1, -1);
        sf::Vector2i dirs[4] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
        
        for (const auto& dir : dirs) {
            int nx = current.x + dir.x;
            int ny = current.y + dir.y;
            
            if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                if (visited.find({nx, ny}) == visited.end()) {
                    char c = grid[ny][nx];
                    if (c == '#' || c == 'E' || c == 'M') {
                        nextStep = {nx, ny};
                        break;
                    }
                }
            }
        }

        if (nextStep.x != -1) {
            current = nextStep;
        } else {
            break;
        }
    }

    return true;
}

void Map::draw(sf::RenderWindow& window) const {
    for (const auto& spot : bridgeSpots) {
        sf::RectangleShape bridge(sf::Vector2f(tileSize, tileSize));
        bridge.setOrigin(tileSize / 2.f, tileSize / 2.f);
        bridge.setPosition(spot);
        bridge.setFillColor(sf::Color(139, 69, 19));
        window.draw(bridge);
    }

    for (size_t i = 0; i < pathPoints.size(); ++i) {
        bool isP1Bridge = false;
        for (const auto& b : bridgeSpots) {
            if (std::abs(b.x - pathPoints[i].x) < 1.f && std::abs(b.y - pathPoints[i].y) < 1.f) {
                isP1Bridge = true;
                break;
            }
        }

        if (i < pathPoints.size() - 1) {
            sf::Vector2f p1 = pathPoints[i];
            sf::Vector2f p2 = pathPoints[i+1];
            
            bool isP2Bridge = false;
            for (const auto& b : bridgeSpots) {
                if (std::abs(b.x - p2.x) < 1.f && std::abs(b.y - p2.y) < 1.f) {
                    isP2Bridge = true;
                    break;
                }
            }

            sf::Vector2f dir = p2 - p1;
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            sf::RectangleShape line(sf::Vector2f(length, tileSize));
            line.setOrigin(0, tileSize / 2.f);
            line.setPosition(p1);
            
            if (isP1Bridge || isP2Bridge) {
                line.setFillColor(sf::Color(160, 82, 45));
            } else {
                line.setFillColor(sf::Color::White);
            }
            
            float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
            line.setRotation(angle);
            window.draw(line);
        }
        
        sf::CircleShape joint(tileSize / 2.f);
        joint.setOrigin(tileSize / 2.f, tileSize / 2.f);
        
        if (isP1Bridge) {
            joint.setFillColor(sf::Color(160, 82, 45));
        } else {
            joint.setFillColor(sf::Color::White);
        }
        
        joint.setPosition(pathPoints[i]);
        window.draw(joint);
    }

    for (const auto& spot : towerSpots) {
        sf::RectangleShape towerBase(sf::Vector2f(tileSize - 4.f, tileSize - 4.f));
        towerBase.setOrigin((tileSize - 4.f) / 2.f, (tileSize - 4.f) / 2.f);
        towerBase.setPosition(spot);
        towerBase.setFillColor(sf::Color(100, 100, 100));
        towerBase.setOutlineThickness(2.f);
        towerBase.setOutlineColor(sf::Color(150, 150, 150));
        window.draw(towerBase);
    }

    for (const auto& spot : storeSpots) {
        sf::RectangleShape storeBase(sf::Vector2f(tileSize * 3.f, tileSize * 3.f));
        storeBase.setOrigin(tileSize * 1.5f, tileSize * 1.5f);
        storeBase.setPosition(spot);
        storeBase.setFillColor(sf::Color::Yellow);
        storeBase.setOutlineThickness(3.f);
        storeBase.setOutlineColor(sf::Color(218, 165, 32));
        window.draw(storeBase);
    }
}
