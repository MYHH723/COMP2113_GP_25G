// game_save.cpp — persistence for Game (save/load); isolated from game loop logic.
#include "persistence.h"
#include "savegame.h"
#include "game.h"
#include "player.h"
#include "mapgenerator.h"
#include "room.h"
#include "monster.h"
#include "trap.h"
#include "utils.h"
#include "merchant.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <string>
#include <exception>
#include <memory>

namespace {

bool jsonBool(const json& j, const char* key, bool fallback) {
    if (!j.contains(key)) return fallback;
    const auto& v = j.at(key);
    if (v.is_boolean()) return v.get<bool>();
    if (v.is_number_integer()) return v.get<int>() != 0;
    return fallback;
}

int jsonIntClamped(const json& j, const char* key, int fallback, int lo, int hi) {
    if (!j.contains(key) || !j.at(key).is_number_integer()) return fallback;
    int x = j.at(key).get<int>();
    return std::max(lo, std::min(hi, x));
}

int jsonIntMin(const json& j, const char* key, int fallback, int lo) {
    if (!j.contains(key) || !j.at(key).is_number_integer()) return fallback;
    int x = j.at(key).get<int>();
    return std::max(lo, x);
}

std::string jsonString(const json& j, const char* key, const std::string& fallback) {
    if (!j.contains(key) || !j.at(key).is_string()) return fallback;
    return j.at(key).get<std::string>();
}

} // namespace

void Game::saveGame(bool verbose) {
    if (!player) {
        std::cerr << "Error: Could not save (no player).\n";
        return;
    }

    json saveData;
    saveData["saveVersion"] = kSaveVersion;
    saveData["Seed"] = seed;
    saveData["difficulty"] = difficulty;
    saveData["totalRooms"] = totalRooms;
    saveData["currentRoom"] = currentRoomIndex;
    saveData["playerWin"] = playerWin;
    saveData["playerName"] = playerName;
    saveData["player"] = player->toJson();
    saveData["rooms"] = json::array();
    for (const auto& roomPtr : rooms) {
        Room* room = roomPtr.get();
        if (room == nullptr) continue;
        json roomData;
        roomData["id"] = room->getRoomId();
        roomData["type"] = static_cast<int>(room->getRoomType());
        roomData["cleared"] = room->isRoomCleared();

        roomData["monsters"] = json::array();
        for (const auto& monsterPtr : room->getMonsters()) {
            Monster* monster = monsterPtr.get();
            if (monster == nullptr) continue;
            json monsterData;
            monsterData["hp"] = monster->getHP();
            monsterData["alive"] = monster->get_isAlive();
            roomData["monsters"].push_back(monsterData);
        }

        roomData["traps"] = json::array();
        for (const auto& trapPtr : room->getTraps()) {
            Trap* trap = trapPtr.get();
            if (trap == nullptr) continue;
            json trapData;
            trapData["active"] = trap->get_isActive();
            trapData["triggerCount"] = trap->getTriggerCount();
            roomData["traps"].push_back(trapData);
        }

        saveData["rooms"].push_back(roomData);
    }

    if (!ensureDataDirectory()) {
        std::cerr << "Error: Could not create data directory.\n";
        return;
    }

    const std::string tmpPath = "data/save.json.tmp";
    const std::string finalPath = getSaveFilePath();

    try {
        std::ofstream file(tmpPath, std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Error: Could not save game.\n";
            return;
        }
        file << saveData.dump(4);
        file.flush();
        file.close();
#ifdef _WIN32
        std::remove(finalPath.c_str());
#endif
        if (std::rename(tmpPath.c_str(), finalPath.c_str()) != 0) {
            std::cerr << "Error: Could not finalize save file.\n";
        } else if (verbose) {
            std::cout << "Game saved.\n";
        }
    } catch (const std::exception&) {
        std::cerr << "Error: Could not save game.\n";
    }
}

Game::LoadResult Game::loadGame() {
    std::ifstream file(getSaveFilePath());
    if (!file.is_open()) {
        std::cout << "No save file found.\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    json saveData;
    try {
        file >> saveData;
    } catch (const std::exception&) {
        std::cout << "Save file is corrupted or invalid JSON.\n";
        file.close();
        isRunning = false;
        return LoadResult::Failed;
    }
    file.close();

    if (!saveData.is_object()) {
        std::cout << "Save file is corrupted (invalid root).\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    try {
        if (saveData.contains("Seed") && saveData["Seed"].is_number_integer())
            seed = saveData["Seed"].get<int>();

        difficulty = jsonIntClamped(saveData, "difficulty", 1, 0, 2);
        totalRooms = jsonIntMin(saveData, "totalRooms", 15, 1);
        totalRooms = std::min(totalRooms, 500);

        currentRoomIndex = jsonIntMin(saveData, "currentRoom", 0, 0);
        playerWin = jsonBool(saveData, "playerWin", false);
        playerName = jsonString(saveData, "playerName", "Hero");

        if (currentRoomIndex > totalRooms) currentRoomIndex = totalRooms;
    } catch (const std::exception&) {
        std::cout << "Save file is corrupted (bad header fields).\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    if (playerWin || currentRoomIndex >= totalRooms) {
        isRunning = false;
        std::cout << "This save is already completed.\n";
        pause();
        return LoadResult::AlreadyCompleted;
    }

    applyDifficultyScaling();
    seedGameRandom(static_cast<unsigned>(seed));
    seedMerchantCatalog(difficulty, seed);

    if (!saveData.contains("player") || !saveData["player"].is_object()) {
        std::cout << "Save file is missing player data.\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    try {
        std::unique_ptr<Player> loaded(new Player(playerName));
        loaded->fromJson(saveData["player"]);
        player = std::move(loaded);
    } catch (const std::exception&) {
        std::cout << "Save player data is corrupted.\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    rooms.clear();
    mapGen.reset(new MapGenerator(seed));
    mapGen->initMapGenerator(totalRooms, difficulty);
    mapGen->generateMap();
    rooms = mapGen->releaseRooms();

    // v3+: room layout always from current map rules (seed); restore progress only.
    if (saveData.contains("rooms") && saveData["rooms"].is_array()) {
        for (const auto& roomData : saveData["rooms"]) {
            if (!roomData.is_object()) continue;
            int id = -1;
            try {
                if (roomData.contains("id") && roomData["id"].is_number_integer())
                    id = roomData["id"].get<int>();
            } catch (const std::exception&) {
                continue;
            }
            if (id < 1 || id > static_cast<int>(rooms.size())) continue;
            Room* room = rooms[static_cast<size_t>(id - 1)].get();
            if (room == nullptr) continue;

            bool cleared = false;
            if (roomData.contains("cleared")) {
                if (roomData["cleared"].is_boolean())
                    cleared = roomData["cleared"].get<bool>();
                else if (roomData["cleared"].is_number_integer())
                    cleared = roomData["cleared"].get<int>() != 0;
            }
            room->setCleared(cleared);

            if (roomData.contains("monsters") && roomData["monsters"].is_array()) {
                const auto& monsters = room->getMonsters();
                size_t mi = 0;
                for (const auto& monsterData : roomData["monsters"]) {
                    if (mi >= monsters.size() || monsters[mi] == nullptr) break;
                    if (!monsterData.is_object()) {
                        ++mi;
                        continue;
                    }
                    Monster* monster = monsters[mi].get();
                    if (monsterData.contains("hp") && monsterData["hp"].is_number_integer()) {
                        monster->setHP(monsterData["hp"].get<int>());
                    }
                    if (monsterData.contains("alive")) {
                        if (monsterData["alive"].is_boolean()) {
                            monster->set_isAlive(monsterData["alive"].get<bool>());
                        } else if (monsterData["alive"].is_number_integer()) {
                            monster->set_isAlive(monsterData["alive"].get<int>() != 0);
                        }
                    }
                    ++mi;
                }
            }

            if (roomData.contains("traps") && roomData["traps"].is_array()) {
                const auto& traps = room->getTraps();
                size_t ti = 0;
                for (const auto& trapData : roomData["traps"]) {
                    if (ti >= traps.size() || traps[ti] == nullptr) break;
                    if (!trapData.is_object()) {
                        ++ti;
                        continue;
                    }
                    Trap* trap = traps[ti].get();
                    if (trapData.contains("active")) {
                        if (trapData["active"].is_boolean()) {
                            trap->setActive(trapData["active"].get<bool>());
                        } else if (trapData["active"].is_number_integer()) {
                            trap->setActive(trapData["active"].get<int>() != 0);
                        }
                    }
                    if (trapData.contains("triggerCount") &&
                        trapData["triggerCount"].is_number_integer()) {
                        trap->setTriggerCount(trapData["triggerCount"].get<int>());
                    }
                    ++ti;
                }
            }
        }
    }

    isRunning = true;
    pendingNewGameWelcome = false;
    std::cout << "Game loaded. Welcome back, " << playerName << "!\n";
    pause();
    return LoadResult::Loaded;
}

void Game::pause() {
    waitForEnter();
}
