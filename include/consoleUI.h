// Header guard to prevent multiple inclusions of this header file
#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

// Standard I/O library for console output operations
#include <iostream>

// Standard dynamic array container for data storage
#include <vector>

// Standard string library for text handling
#include <string>

// Include Player class definition
#include "player.h"

// ConsoleUI class: Provides static UI functions for displaying game information in console
class ConsoleUI
{
public:
    // Display the current status (HP, stats, etc.) of the given player
    static void showPlayerStatus(const Player &player);

    // Display all items in the player's inventory list
    static void showInventory(const Player &player);

    // Display the main menu interface of the game
    static void showMainMenu();
};

#endif
