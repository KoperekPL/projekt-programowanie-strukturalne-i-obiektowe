#include "Map.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <set>
#include <utility>
#include <vector>

Map::Map(float tSize) : tileSize(tSize), hasBg(false) {}

bool Map::loadFromFile(const std::string& filename) {
    const std::string base = PROJECT_DIR "assets/textures/";

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
    healerSpots.clear();

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
            } else if (cell == 'H') {
                healerSpots.push_back({x * tileSize + tileSize / 2.f, y * tileSize + tileSize / 2.f});
            }
        }
    }

    pathPointsA.clear();
    pathPointsB.clear();

    if (startPos.x == -1) {
        std::cerr << "Brak punktu startowego 'S' na mapie!\n";
        return false;
    }

    // --- Helpers ---
    auto toWorld = [&](sf::Vector2i c) -> sf::Vector2f {
        return sf::Vector2f(c.x * tileSize + tileSize / 2.f, c.y * tileSize + tileSize / 2.f);
    };

    auto isPathCell = [&](int x, int y) -> bool {
        if (x < 0 || x >= cols || y < 0 || y >= rows) return false;
        char c = grid[y][x];
        return (c == '#' || c == 'E' || c == 'M' || c == 'S' || c == 'D');
    };

    auto findNext = [&](sf::Vector2i cur, const std::set<std::pair<int,int>>& vis) -> std::vector<sf::Vector2i> {
        std::vector<sf::Vector2i> res;
        sf::Vector2i dirs[4] = {{0,-1},{0,1},{-1,0},{1,0}};
        for (auto& d : dirs) {
            int nx = cur.x + d.x;
            int ny = cur.y + d.y;
            if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
                if (!vis.count({nx, ny}) && isPathCell(nx, ny)) {
                    res.push_back({nx, ny});
                }
            }
        }
        return res;
    };

    // Sprawdza, czy idąc od 'start' (bez powrotu do komórek z 'blocked')
    // można dojść do 'E'
    auto canReachE = [&](sf::Vector2i start, std::set<std::pair<int,int>> blocked) -> bool {
        if (blocked.count({start.x, start.y})) return false;
        std::vector<sf::Vector2i> stack = {start};
        std::set<std::pair<int,int>> localVis = blocked;
        while (!stack.empty()) {
            sf::Vector2i c = stack.back();
            stack.pop_back();
            if (localVis.count({c.x, c.y})) continue;
            localVis.insert({c.x, c.y});
            if (grid[c.y][c.x] == 'E') return true;
            for (auto& n : findNext(c, localVis)) stack.push_back(n);
        }
        return false;
    };

    // Sprawdza, czy idąc od 'start' do E (bez powrotu do 'blocked') trasa przechodzi przez 'D'
    auto pathContainsD = [&](sf::Vector2i start, std::set<std::pair<int,int>> blocked) -> bool {
        if (blocked.count({start.x, start.y})) return false;
        std::vector<sf::Vector2i> stack = {start};
        std::set<std::pair<int,int>> localVis = blocked;
        while (!stack.empty()) {
            sf::Vector2i c = stack.back();
            stack.pop_back();
            if (localVis.count({c.x, c.y})) continue;
            localVis.insert({c.x, c.y});
            if (grid[c.y][c.x] == 'D') return true;
            if (grid[c.y][c.x] == 'E') continue; // E bez D na tej trasie - nie liczy się
            for (auto& n : findNext(c, localVis)) stack.push_back(n);
        }
        return false;
    };

    // --- Build path (with optional single fork) ---
    auto buildPath = [&](int forkChoice) -> std::vector<sf::Vector2f> {
        std::vector<sf::Vector2i> result;
        std::set<std::pair<int,int>> vis;
        sf::Vector2i c = startPos;
        bool usedFork = false;

        while (true) {
            vis.insert({c.x, c.y});
            result.push_back(c);

            if (grid[c.y][c.x] == 'E') break;

            auto nxt = findNext(c, vis);
            if (nxt.empty()) break;

            // Jeśli stoimy na 'D', musimy iść do kolejnego 'D' (jeśli istnieje wśród sąsiadów)
            if (grid[c.y][c.x] == 'D') {
                bool foundD = false;
                for (auto& n : nxt) {
                    if (grid[n.y][n.x] == 'D') {
                        c = n;
                        foundD = true;
                        break;
                    }
                }
                if (foundD) continue;
            }

            if (!usedFork && nxt.size() >= 2) {
                // Filtrujemy tylko te sąsiady, z których realnie da się dojść do E
                std::vector<sf::Vector2i> viable;
                for (auto& n : nxt) {
                    if (canReachE(n, vis)) viable.push_back(n);
                }

                if (viable.size() >= 2) {
                    // Jeśli dokładnie jedna z gałęzi prowadzi przez D, wymuszamy ją
                    // (nie losujemy na tym forku - obie wersje muszą przejść przez D)
                    std::vector<int> dBranches;
                    for (size_t i = 0; i < viable.size(); ++i) {
                        if (pathContainsD(viable[i], vis)) dBranches.push_back((int)i);
                    }

                    if (dBranches.size() == 1) {
                        c = viable[dBranches[0]];
                        // usedFork zostaje false - kolejne forki dalej mogą losować
                        continue;
                    }

                    // Prawdziwy fork (żadna gałąź nie wymusza D, albo obie prowadzą przez D)
                    usedFork = true;
                    size_t idx = (size_t)forkChoice < viable.size() ? (size_t)forkChoice : viable.size() - 1;
                    c = viable[idx];
                    continue;
                }
                // jeśli tylko jedna gałąź jest "żywa", idziemy nią (to nie był prawdziwy fork)
                if (!viable.empty()) {
                    c = viable[0];
                    continue;
                }
                // żadna nie prowadzi do E - fallback na pierwszego sąsiada
                c = nxt[0];
                continue;
            }

            c = nxt[0];
        }

        std::vector<sf::Vector2f> out;
        for (auto& p : result) out.push_back(toWorld(p));
        return out;
    };

    pathPointsA = buildPath(0);
    pathPointsB = buildPath(1);

    // --- Decorations ---
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
            sf::Sprite tile;
            tile.setTexture(texGrass);
            sf::Vector2u texSize = texGrass.getSize();
            float sx = ts / (float)texSize.x;
            float sy = ts / (float)texSize.y;
            tile.setScale(sx, sy);
            tile.setPosition(x * ts, y * ts);
            window.draw(tile);
        }
    }

    float pathScaleMultiplier = 1.45f;
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < (int)grid[y].size(); ++x) {
            char cell = grid[y][x];
            bool isPath = (cell == '#' || cell == 'S' || cell == 'E' || cell == 'M' || cell == 'D');
            if (!isPath) continue;

            sf::Sprite tile;
            tile.setTexture(texPath);
            sf::Vector2u texSize = texPath.getSize();
            tile.setOrigin(texSize.x / 2.f, texSize.y / 2.f);

            float sx = (ts * pathScaleMultiplier) / (float)texSize.x;
            float sy = (ts * pathScaleMultiplier) / (float)texSize.y;
            tile.setScale(sx, sy);
            tile.setPosition(x * ts + ts / 2.f, y * ts + ts / 2.f);
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
