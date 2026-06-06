#ifndef MAPGENERATOR_H
#define MAPGENERATOR_H

#include "room.h"
#include "types.h"
#include <memory>
#include <vector>

class Room;

class MapGenerator {
private:
    int totalRooms;
    int difficulty;
    std::vector<std::unique_ptr<Room>> generatedRooms;
    int shopFrequency;

    RoomType determineRoomType(int roomNumber);
    std::unique_ptr<Room> buildRoom(int roomId, int roomNumber);

public:
    MapGenerator();
    MapGenerator(int seed);
    ~MapGenerator();

    void initMapGenerator(int numRooms, int diff);
    void generateMap();

    /** Transfer ownership of generated rooms to the caller (e.g. Game). */
    std::vector<std::unique_ptr<Room>> releaseRooms();
    int getTotalRooms() const;
    int getDifficulty() const;
    Room* getRoomById(int roomId);

    bool shouldHaveShop(int roomNumber) const;
};

#endif // MAPGENERATOR_H
