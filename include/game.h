// game.h
// Declaration of the Game class - controls the whole game flow.

#ifndef GAME_H
#define GAME_H

#include <memory>
#include <string>
#include <vector>

class Player;
class MapGenerator;
class Room;
class BattleSystem;
class Shop;

class Game {
public:
    enum class LoadResult {
        Loaded,
        Failed,
        AlreadyCompleted
    };

    enum class RoomCombatResult {
        Cleared,
        Lost,
        Fled,
        Empty
    };

private:
    int seed;
    int difficulty;
    int totalRooms;
    int currentRoomIndex;
    bool isRunning;
    bool playerWin;
    bool pendingNewGameWelcome;

    std::unique_ptr<Player> player;
    std::string playerName;

    std::unique_ptr<MapGenerator> mapGen;
    std::vector<std::unique_ptr<Room>> rooms;

    void generateRooms();
    void applyDifficultyScaling();
    bool triggerRoomTraps(Room* room);
    RoomCombatResult fightRoomMonsters(Room* room);

public:
    Game();
    ~Game();

    void showMainMenu();
    bool selectDifficulty();
    void initGame();
    void saveGame(bool verbose = false);
    LoadResult loadGame();
    void pause();

    void gameLoop();
    void enterNextRoom();

    void checkGameOver();
    void showGameResult();
};

#endif
