#include "mapgenerator.h"
#include "utils.h"

#include <algorithm>

MapGenerator::MapGenerator()
    : totalRooms(0), difficulty(1), shopFrequency(4) {}

MapGenerator::MapGenerator(int /*seed*/)
    : totalRooms(0), difficulty(1), shopFrequency(4) {}

MapGenerator::~MapGenerator() = default;

void MapGenerator::initMapGenerator(int numRooms, int diff) {
    totalRooms = std::max(1, numRooms);
    difficulty = std::max(0, std::min(diff, 2));
    switch (difficulty) {
    case 0: shopFrequency = 4; break;
    case 2: shopFrequency = 3; break;
    default: shopFrequency = 4; break;
    }
}

void MapGenerator::generateMap() {
    generatedRooms.clear();
    generatedRooms.reserve(static_cast<size_t>(totalRooms));

    for (int roomNumber = 1; roomNumber <= totalRooms; ++roomNumber) {
        generatedRooms.push_back(buildRoom(roomNumber, roomNumber));
    }
}

std::unique_ptr<Room> MapGenerator::buildRoom(int roomId, int roomNumber) {
    std::unique_ptr<Room> room(new Room());
    const RoomType type = determineRoomType(roomNumber);
    room->initRoom(roomId, difficulty, type);
    room->generateRoomContent(difficulty);
    room->setHasShop(type == SHOP);
    return room;
}

std::vector<std::unique_ptr<Room>> MapGenerator::releaseRooms() {
    return std::move(generatedRooms);
}

RoomType MapGenerator::determineRoomType(int roomNumber) {
    if (roomNumber >= totalRooms) return BOSS;
    if (roomNumber > 0 && roomNumber < totalRooms && (roomNumber % 5) == 0) return BOSS;
    if (shouldHaveShop(roomNumber)) return SHOP;
    if (getRandom(0, 99) < 12) return TREASURE;
    return NORMAL;
}

int MapGenerator::getTotalRooms() const {
    return totalRooms;
}

int MapGenerator::getDifficulty() const {
    return difficulty;
}

Room* MapGenerator::getRoomById(int roomId) {
    for (const auto& room : generatedRooms) {
        if (room != nullptr && room->getRoomId() == roomId) return room.get();
    }
    return nullptr;
}

bool MapGenerator::shouldHaveShop(int roomNumber) const {
    if (roomNumber <= 1 || roomNumber >= totalRooms) return false;
    if (shopFrequency <= 0) return false;
    if ((roomNumber % 5) == 0) return false;
    return (roomNumber % shopFrequency) == 0;
}
