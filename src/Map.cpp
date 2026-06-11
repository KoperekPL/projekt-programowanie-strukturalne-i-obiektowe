#include "Map.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <utility>

Map::Map(float tSize) : tileSize(tSize), hasBg(false) {}

bool Map::loadFromFile(const std::string& filename) {
    const std::string base = "../../../assets/textures/";

    hasBg = true;
    if (!texGrass.loadFromFile(base + "trawa.png"))   { std::cerr << "Brak trawa.png\n";   hasBg = false; }
    if (!texPath.loadFromFile(base  + "sciezka.png")) { std::cerr << "Brak sciezka.png\n"; hasBg = false; }

    const char* decoFiles[NUM_DECO] = {
        "drzewo.png","krzak1.png","krzak2.png","pien.png",
        "trawa1.png","trawa2.png","trawa3.png","trawa4.png","trawa5.png","trawa6.png"
    };
    for (int i = 0; i < NUM_DECO; ++i) {
        texDeco[i].loadFromFile(base + decoFiles[i]);
        texDeco[i].setSmooth(false);
    }
    texGrass.setSmooth(false);
    texPath.setSmooth(false);

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Nie udalo sie otworzyc mapy: " << filename << std::endl;
        return false;
    }

    grid.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) grid.push_back(line);
    }
    file.close();
    if (grid.empty()) return false;

    int rows = (int)grid.size();
    int cols = (int)grid[0].size();

    sf::Vector2i startPos(-1, -1);
    towerSpots.clear();
    bridgeSpots.clear();
    storeSpots.clear();

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < (int)grid[y].size(); ++x) {
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
        std::cerr << "Brak punktu startowego 'S' na mapie!\n";
        return false;
    }

    sf::Vector2i current = startPos;
    std::set<std::pair<int,int>> visited;

    while (true) {
        visited.insert({current.x, current.y});
        pathPoints.push_back({current.x * tileSize + tileSize / 2.f,
                               current.y * tileSize + tileSize / 2.f});

        if (grid[current.y][current.x] == 'E') break;

        sf::Vector2i nextStep(-1, -1);
        sf::Vector2i dirs[4] = {{0,-1},{0,1},{-1,0},{1,0}};
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
        if (nextStep.x != -1) current = nextStep;
        else break;
    }

    decorations.clear();

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < (int)grid[y].size(); ++x) {
            if (grid[y][x] != '.') continue;

            bool nearTower = false;
            for (int dy = -1; dy <= 1 && !nearTower; ++dy) {
                for (int dx = -1; dx <= 1 && !nearTower; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                        if (grid[ny][nx] == 'T') nearTower = true;
                    }
                }
            }
            if (nearTower) continue;

            unsigned int seed = (unsigned int)(y * 1000 + x) * 2654435761u;
            unsigned int r1 = (seed ^ (seed >> 16)) * 0x45d9f3b;
            unsigned int r2 = (r1   ^ (r1   >> 16)) * 0x45d9f3b;
            unsigned int r3 =  r2   ^ (r2   >> 16);

            if ((r1 % 100) >= 15) continue;

            int decoIdx = (int)(r2 % NUM_DECO);

            float margin = tileSize * 0.2f;
            float inner  = tileSize - 2.f * margin;
            float ox = margin + ((r3 & 0xFFFF) / 65535.f) * inner;
            float oy = margin + ((r3 >> 16)    / 65535.f) * inner;

            DecoInstance di;
            di.texIndex = decoIdx;
            di.position = { x * tileSize + ox, y * tileSize + oy };

            float targetW;
            if (decoIdx == 0)       targetW = tileSize * 1.4f;
            else if (decoIdx <= 2)  targetW = tileSize * 1.0f;
            else if (decoIdx == 3)  targetW = tileSize * 0.6f;
            else                    targetW = tileSize * 0.5f;

            sf::Vector2u ds = texDeco[decoIdx].getSize();
            di.scale = (ds.x > 0) ? (targetW / (float)ds.x) : 1.f;

            decorations.push_back(di);
        }
    }

    return true;
}

void Map::draw(sf::RenderWindow& window) const {
    int rows = (int)grid.size();
    if (rows == 0) return;

    float ts = tileSize;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < (int)grid[y].size(); ++x) {
            char cell = grid[y][x];
            bool isPath = (cell == '#' || cell == 'S' || cell == 'E' || cell == 'M');

            sf::Sprite tile;
            if (isPath) {
                tile.setTexture(texPath);
            } else {
                tile.setTexture(texGrass);
            }

            sf::Vector2u texSize = isPath ? texPath.getSize() : texGrass.getSize();
            float sx = ts / (float)texSize.x;
            float sy = ts / (float)texSize.y;
            tile.setScale(sx, sy);
            tile.setPosition(x * ts, y * ts);
            window.draw(tile);
        }
    }

    for (const auto& di : decorations) {
        sf::Sprite deco;
        deco.setTexture(texDeco[di.texIndex]);
        sf::Vector2u ds = texDeco[di.texIndex].getSize();
        deco.setOrigin(ds.x / 2.f, ds.y / 2.f);
        deco.setScale(di.scale, di.scale);
        deco.setPosition(di.position);
        window.draw(deco);
    }
}
