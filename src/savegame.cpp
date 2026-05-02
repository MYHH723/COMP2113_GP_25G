// savegame.cpp
#include "savegame.h"
#include "game.h"
#include "player.h"
#include "mapgenerator.h"
#include "room.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <limits>


void Game::saveGame() {
    json saveData;
    saveData["Seed"] = seed;
    saveData["difficulty"] = difficulty;
    saveData["totalRooms"] = totalRooms;
    saveData["currentRoom"] = currentRoomIndex;
    saveData["playerWin"] = playerWin;
    saveData["playerName"] = playerName;
    saveData["player"] = player->toJson();   // Player class must implement toJson()
    saveData["rooms"] = json::array();
    for (Room* room : rooms) {
        if (room == nullptr) continue;
        json roomData;
        roomData["id"] = room->getRoomId();
        roomData["type"] = static_cast<int>(room->getRoomType());
        roomData["cleared"] = room->isRoomCleared();
        saveData["rooms"].push_back(roomData);
    }

    // Ensure save directory exists before opening save file.
#ifdef _WIN32
    std::system("if not exist data mkdir data >nul 2>nul");
#else
    std::system("mkdir -p data >/dev/null 2>&1");
#endif

    std::ofstream file("data/save.json");
    if (file.is_open()) {
        file << saveData.dump(4);   // pretty print with 4 spaces
        file.close();
        std::cout << "Game saved.\n";
    } else {
        std::cerr << "Error: Could not save game.\n";
    }
}

void Game::loadGame() {
    std::ifstream file("data/save.json");
    if (!file.is_open()) {
        std::cout << "No save file found.\n";
        isRunning = false;
        return;
    }

    json saveData;
    file >> saveData;
    file.close();

    // Restore game state
    seed = saveData.value("Seed", seed);
    difficulty = saveData["difficulty"];
    totalRooms = saveData["totalRooms"];
    currentRoomIndex = saveData["currentRoom"];
    playerWin = saveData["playerWin"];
    playerName = saveData["playerName"];

    // Completed saves should return to menu instead of re-entering game loop.
    if (playerWin || currentRoomIndex >= totalRooms) {
        isRunning = false;
        std::cout << "This save is already completed.\n";
        pause();
        return;
    }

    // Recreate player and load its data
    delete player;
    player = new Player(playerName);
    player->fromJson(saveData["player"]);

    // Recreate map generator and rooms
    delete mapGen;
    for (Room* r : rooms) delete r;
    rooms.clear();
    mapGen = new MapGenerator(seed);
    mapGen->initMapGenerator(totalRooms, difficulty);
    mapGen->generateMap();   // Note: room contents may differ from saved state
    rooms = mapGen->getGeneratedRooms();

    // Restore room-level state that should persist across saves.
    if (saveData.contains("rooms") && saveData["rooms"].is_array()) {
        for (const auto& roomData : saveData["rooms"]) {
            int id = roomData.value("id", -1);
            if (id < 1 || id > static_cast<int>(rooms.size())) continue;
            Room* room = rooms[id - 1];
            if (room == nullptr) continue;

            int typeInt = roomData.value("type", static_cast<int>(room->getRoomType()));
            if (typeInt >= static_cast<int>(NORMAL) && typeInt <= static_cast<int>(TREASURE)) {
                RoomType restoredType = static_cast<RoomType>(typeInt);
                room->setRoomType(restoredType);
                room->setHasShop(restoredType == SHOP);
            }
            room->setCleared(roomData.value("cleared", false));
        }
    }

    isRunning = true;
    std::cout << "Game loaded. Welcome back, " << playerName << "!\n";
    pause();
}

void Game::pause() {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}