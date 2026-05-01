// mapgenerator.cpp
#include "mapgenerator.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

MapGenerator::MapGenerator()
    : totalRooms(0), difficulty(1), shopFrequency(4) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

MapGenerator::MapGenerator(int seed)
    : totalRooms(0), difficulty(1), shopFrequency(4) {
    std::srand(static_cast<unsigned>(seed));
}

MapGenerator::~MapGenerator() {
    // Rooms are owned and deleted by Game.
    generatedRooms.clear();
}

void MapGenerator::initMapGenerator(int numRooms, int diff) {
    totalRooms = std::max(1, numRooms);
    difficulty = std::max(0, std::min(diff, 2));
    shopFrequency = 5 - difficulty; // easy=5, normal=4, hard=3
    if (shopFrequency < 2) shopFrequency = 2;
}

void MapGenerator::generateMap() {
    generatedRooms.clear();
    generatedRooms.reserve(static_cast<size_t>(totalRooms));

    for (int roomNumber = 1; roomNumber <= totalRooms; ++roomNumber) {
        Room* room = generateRoom(roomNumber, roomNumber);
        generatedRooms.push_back(room);
    }
}

Room* MapGenerator::generateRoom(int roomId, int roomNumber) {
    Room* room = new Room();

    RoomType type = NORMAL;
    if (roomNumber >= totalRooms) {
        // Requirement: last room is always a boss room.
        type = BOSS;
    } else {
        // Random room type generation for non-final rooms.
        // We keep NORMAL as the most frequent outcome.
        int roll = std::rand() % 100; // 0-99
        if (roll < 65) {
            type = NORMAL;     // 65%
        } else if (roll < 80) {
            type = SHOP;       // 15%
        } else if (roll < 92) {
            type = TREASURE;   // 12%
        } else {
            type = BOSS;       // 8%
        }
    }

    room->initRoom(roomId, difficulty, type);
    room->generateRoomContent(difficulty);
    room->setHasShop(type == SHOP);
    return room;
}

void MapGenerator::determineRoomType(Room* room, int roomNumber) {
    if (room == nullptr) return;
    RoomType type = determineRoomType(roomNumber);
    room->setRoomType(type);
    room->setHasShop(type == SHOP);
}

RoomType MapGenerator::determineRoomType(int roomNumber) {
    if (roomNumber == totalRooms) return TREASURE;
    if (roomNumber > 0 && roomNumber < totalRooms && (roomNumber % 5) == 0) return BOSS;
    if (shouldHaveShop(roomNumber)) return SHOP;
    return NORMAL;
}

std::vector<Room*> MapGenerator::getGeneratedRooms() const {
    return generatedRooms;
}

int MapGenerator::getTotalRooms() const {
    return totalRooms;
}

int MapGenerator::getDifficulty() const {
    return difficulty;
}

Room* MapGenerator::getRoomById(int roomId) {
    for (Room* room : generatedRooms) {
        if (room != nullptr && room->getRoomId() == roomId) return room;
    }
    return nullptr;
}

bool MapGenerator::shouldHaveShop(int roomNumber) {
    if (roomNumber <= 1 || roomNumber >= totalRooms) return false;
    if (shopFrequency <= 0) return false;
    return (roomNumber % shopFrequency) == 0;
}

int MapGenerator::getRandomMonsterCount(int diff) {
    int clamped = std::max(0, std::min(diff, 2));
    switch (clamped) {
        case 0: return 1 + (std::rand() % 2); // 1-2
        case 1: return 1 + (std::rand() % 3); // 1-3
        default: return 2 + (std::rand() % 3); // 2-4
    }
}

int MapGenerator::getRandomTrapCount(int diff) {
    int clamped = std::max(0, std::min(diff, 2));
    switch (clamped) {
        case 0: return std::rand() % 2; // 0-1
        case 1: return std::rand() % 3; // 0-2
        default: return 1 + (std::rand() % 3); // 1-3
    }
}

int MapGenerator::countRoomsByType(RoomType type) const {
    int count = 0;
    for (Room* room : generatedRooms) {
        if (room != nullptr && room->getRoomType() == type) ++count;
    }
    return count;
}