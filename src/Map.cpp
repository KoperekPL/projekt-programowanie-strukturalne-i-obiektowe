#include "Map.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <utility>

Map::Map(float tSize) : tileSize(tSize), hasBg(false) {}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc mapy: " << filename << std::endl;
        return false;
    }

    if (bgTexture.loadFromFile("../../../assets/textures/mapa.png")) {
        bgSprite.setTexture(bgTexture);
        hasBg = true;
    } else {
        std::cerr << "Nie udalo sie zaladowac tła: ../../../assets/textures/mapa.png\n";
    }

    this->grid.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            this->grid.push_back(line);
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
    sf::RectangleShape bg(sf::Vector2f(1280.f, 720.f));
    bg.setFillColor(sf::Color(34, 139, 34));
    window.draw(bg);

    if (!hasBg) return;

    int ts = static_cast<int>(tileSize);
    sf::IntRect pathRect(200, 200, ts, ts);
    float pathThickness = tileSize * 3.0f;

    for (size_t i = 0; i < pathPoints.size(); ++i) {
        if (i < pathPoints.size() - 1) {
            sf::Vector2f p1 = pathPoints[i];
            sf::Vector2f p2 = pathPoints[i+1];
            
            sf::Vector2f dir = p2 - p1;
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            sf::RectangleShape line(sf::Vector2f(length, pathThickness));
            line.setOrigin(0, pathThickness / 2.f);
            line.setPosition(p1);
            line.setTexture(&bgTexture);
            line.setTextureRect(pathRect);
            
            float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159265f;
            line.setRotation(angle);
            window.draw(line);
        }
        
        sf::CircleShape joint(pathThickness / 2.f);
        joint.setOrigin(pathThickness / 2.f, pathThickness / 2.f);
        joint.setTexture(&bgTexture);
        joint.setTextureRect(pathRect);
        joint.setPosition(pathPoints[i]);
        window.draw(joint);
    }
}
