// savegame.cpp
#include "savegame.h"
#include "game.h"
#include "player.h"
#include "mapgenerator.h"
#include "room.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <string>
#include <exception>

namespace {
/**
 * @brief Safely get a boolean value from JSON
 * @param j JSON object
 * @param key Target key
 * @param fallback Default value
 * @return Parsed boolean
 */

bool jsonBool(const json& j, const char* key, bool fallback) {
    if (!j.contains(key)) return fallback;
    const auto& v = j.at(key);
    if (v.is_boolean()) return v.get<bool>();
    if (v.is_number_integer()) return v.get<int>() != 0;
    return fallback;
}
/**
 * @brief Get integer from JSON with clamp range
 * @param j JSON object
 * @param key Target key
 * @param fallback Default value
 * @param lo Minimum value
 * @param hi Maximum value
 * @return Clamped integer
 */
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
/**
 * @brief Saves current game state to JSON file
 * Creates data directory and writes save atomically
 * @return None
 */
void Game::saveGame() {
    if (!player) {
        std::cerr << "Error: Could not save (no player).\n";
        return;
    }

    json saveData;
    saveData["Seed"] = seed;
    saveData["difficulty"] = difficulty;
    saveData["totalRooms"] = totalRooms;
    saveData["currentRoom"] = currentRoomIndex;
    saveData["playerWin"] = playerWin;
    saveData["playerName"] = playerName;
    saveData["player"] = player->toJson();
    saveData["rooms"] = json::array();
    for (Room* room : rooms) {
        if (room == nullptr) continue;
        json roomData;
        roomData["id"] = room->getRoomId();
        roomData["type"] = static_cast<int>(room->getRoomType());
        roomData["cleared"] = room->isRoomCleared();
        saveData["rooms"].push_back(roomData);
    }

#ifdef _WIN32
    if (std::system("if not exist data mkdir data >nul 2>nul") != 0) {
        /* best-effort; save may still work if data exists */
    }
#else
    if (std::system("mkdir -p data >/dev/null 2>&1") != 0) {
        /* best-effort */
    }
#endif

    const std::string tmpPath = "data/save.json.tmp";
    const std::string finalPath = "data/save.json";

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
        } else {
            std::cout << "Game saved.\n";
        }
    } catch (const std::exception&) {
        std::cerr << "Error: Could not save game.\n";
    }
}

/**
 * @brief Loads game from save file
 * Restores player, map, rooms, and progress
 * @return LoadResult: Loaded / Failed / AlreadyCompleted
 */
Game::LoadResult Game::loadGame() {
    std::ifstream file("data/save.json");
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

    if (!saveData.contains("player") || !saveData["player"].is_object()) {
        std::cout << "Save file is missing player data.\n";
        isRunning = false;
        return LoadResult::Failed;
    }

    Player* loaded = nullptr;
    try {
        loaded = new Player(playerName);
        loaded->fromJson(saveData["player"]);
    } catch (const std::exception&) {
        delete loaded;
        std::cout << "Save player data is corrupted.\n";
        isRunning = false;
        return LoadResult::Failed;
    }
    delete player;
    player = loaded;

    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();
    mapGen = new MapGenerator(seed);
    mapGen->initMapGenerator(totalRooms, difficulty);
    mapGen->generateMap();
    rooms = mapGen->getGeneratedRooms();

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
            Room* room = rooms[static_cast<size_t>(id - 1)];
            if (room == nullptr) continue;

            int typeInt = static_cast<int>(room->getRoomType());
            try {
                if (roomData.contains("type") && roomData["type"].is_number_integer())
                    typeInt = roomData["type"].get<int>();
            } catch (const std::exception&) {
                typeInt = static_cast<int>(room->getRoomType());
            }
            if (typeInt >= static_cast<int>(NORMAL) && typeInt <= static_cast<int>(TREASURE)) {
                RoomType restoredType = static_cast<RoomType>(typeInt);
                room->setRoomType(restoredType);
                room->setHasShop(restoredType == SHOP);
            }
            bool cleared = false;
            if (roomData.contains("cleared")) {
                if (roomData["cleared"].is_boolean())
                    cleared = roomData["cleared"].get<bool>();
                else if (roomData["cleared"].is_number_integer())
                    cleared = roomData["cleared"].get<int>() != 0;
            }
            room->setCleared(cleared);
        }
    }

    isRunning = true;
    pendingNewGameWelcome = false;
    std::cout << "Game loaded. Welcome back, " << playerName << "!\n";
    pause();
    return LoadResult::Loaded;
}
/**
 * @brief Pauses game and waits for user input
 * @return None
 */
void Game::pause() {
    waitForEnter();
}
