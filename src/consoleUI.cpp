#include <iostream>
#include <limits>
#include "consoleUI.h"
#include "utils.h"

void ConsoleUI::showMainMenu()
{
    std::cout << "=============================\n";
    std::cout << " Sword Magic Dungeon Adventure\n";
    std::cout << "=============================\n";
    std::cout << "Press ENTER to start...\n" << std::flush;
    std::cin.clear();
    std::streambuf* sb = std::cin.rdbuf();
    if (sb && sb->in_avail() > 0) {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::cin.get();
}

void ConsoleUI::showPlayerStatus(const Player &player)
{
    std::cout << "\n=== Player Status ===\n";
    std::cout << "ATK: " << formatFixed2(player.get_ATK()) << "\n";
    std::cout << "DEF: " << formatFixed2(player.get_DEF()) << "\n";
    std::cout << "HP: " << formatFixed2(player.get_HP()) << "\n";
    std::cout << "EXP: " << formatFixed2(player.get_EXP()) << "\n";
    std::cout << "Money: " << formatFixed2(player.get_Money()) << "\n";
    std::cout << "Alive: " << (player.get_isAlive() ? "Yes" : "No") << "\n";
    std::cout << "Score: " << formatFixed2(player.get_score()) << "\n";
}

void ConsoleUI::showInventory(const Player &player)
{
    std::cout << "\n=== Inventory ===\n";

    const auto &items = player.get_all_items();

    if (items.empty())
    {
        std::cout << "(empty)\n";
        return;
    }

    for (const auto &item : items)
    {
        std::cout << "- " << item << "\n";
    }
}