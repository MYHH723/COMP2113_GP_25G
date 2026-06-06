#include <iostream>
#include <limits>
#include <vector>
#include "consoleUI.h"
#include "utils.h"
#include "item.h"

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
    std::cout << "Level: " << player.get_Level() << "\n";
    std::cout << "ATK: " << formatFixed2(player.get_ATK());
    if (player.isDualWielding()) std::cout << " (dual-wield)";
    std::cout << "\n";
    std::cout << "DEF: " << formatFixed2(player.get_DEF());
    if (player.isFullLayeredArmor()) std::cout << " (triple armor)";
    std::cout << "\n";
    std::cout << "HP: " << formatFixed2(player.get_HP()) << " / "
              << formatFixed2(player.get_maxHP()) << "\n";
    std::cout << "EXP: " << formatFixed2(player.get_EXP()) << "\n";
    std::cout << "Money: " << formatFixed2(player.get_Money()) << "\n";
    std::cout << "Poisoned: " << (player.get_isPoisoned() ? "Yes" : "No") << "\n";
    std::cout << "Alive: " << (player.get_isAlive() ? "Yes" : "No") << "\n";
    std::cout << "Battle Score: " << formatFixed2(player.get_battleScore()) << "\n";
    std::cout << "Overall Rating: " << formatFixed2(player.get_score()) << "\n";

    bool anyEquipped = false;
    for (const char* slot : {SLOT_WEAPON, SLOT_WEAPON2, SLOT_ARMOR, SLOT_ARMOR2, SLOT_ARMOR3}) {
        const int id = player.getEquippedId(slot);
        if (id == 0) continue;
        if (!anyEquipped) {
            std::cout << "\n--- Equipped ---\n";
            anyEquipped = true;
        }
        Item item(id);
        std::cout << slot << ": " << item.getName()
                  << " (+" << static_cast<int>(item.getEffectValue()) << ")\n";
    }
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

    int index = 1;
    for (const int itemId : items)
    {
        Item item(itemId);
        std::cout << index << ". " << item.getName()
                  << " [" << static_cast<int>(item.getRarity()) << "] "
                  << "(+" << static_cast<int>(item.getEffectValue()) << ")\n";
        ++index;
    }
}

namespace {

void runInventoryUseMenu(Player &player) {
    while (true) {
        clearScreen();
        ConsoleUI::showPlayerStatus(player);
        std::cout << "\n=== Use / Equip ===\n";
        const auto &items = player.get_all_items();
        if (items.empty()) {
            std::cout << "(empty)\n";
            waitForEnter();
            return;
        }

        std::vector<int> ids(items.begin(), items.end());
        for (size_t i = 0; i < ids.size(); ++i) {
            Item item(ids[i]);
            std::cout << (i + 1) << ". " << item.getName();
            if (item.getType() == POTION) std::cout << " (drink)";
            else if (item.getType() == WEAPON) std::cout << " (equip)";
            else if (item.getType() == ARMOR) std::cout << " (equip)";
            std::cout << "\n";
        }
        std::cout << "0. Back\nSelect: " << std::flush;

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            discardRestOfLine();
            continue;
        }
        discardRestOfLine();
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(ids.size())) {
            std::cout << "Invalid.\n";
            waitForEnter();
            continue;
        }

        const int itemId = ids[static_cast<size_t>(choice - 1)];
        Item item(itemId);
        if (item.getType() == POTION) {
            player.usePotionFromInventory(itemId);
        } else if (item.getType() == WEAPON || item.getType() == ARMOR) {
            if (player.equipFromInventory(itemId)) {
                std::cout << "[Inventory] Equipped " << item.getName() << ".\n";
                const int owned = player.countOwnedItems(itemId);
                if (owned > 1) {
                    std::cout << "[Inventory] You own " << owned << "x " << item.getName() << ".\n";
                }
                if (player.isDualWielding()) {
                    std::cout << "[Inventory] Dual-wield active (armor disabled).\n";
                }
                if (player.isFullLayeredArmor()) {
                    std::cout << "[Inventory] Triple armor active (single sword only).\n";
                }
            } else {
                std::cout << "[Inventory] Could not equip (check rules or backpack space).\n";
            }
        }
        waitForEnter();
    }
}

} // namespace

void ConsoleUI::runUnequipMenu(Player &player) {
    struct SlotInfo {
        const char* key;
        const char* label;
    };
    const SlotInfo slots[] = {
        {SLOT_WEAPON, "Main Hand (sword)"},
        {SLOT_WEAPON2, "Off Hand (sword)"},
        {SLOT_ARMOR, "Armor layer 1"},
        {SLOT_ARMOR2, "Armor layer 2"},
        {SLOT_ARMOR3, "Armor layer 3"},
    };

    while (true) {
        clearScreen();
        ConsoleUI::showPlayerStatus(player);
        std::cout << "\n=== Unequip ===\n";

        std::vector<const char*> activeSlots;
        for (const SlotInfo& slot : slots) {
            const int id = player.getEquippedId(slot.key);
            if (id == 0) continue;
            Item item(id);
            activeSlots.push_back(slot.key);
            std::cout << activeSlots.size() << ". " << slot.label << ": "
                      << item.getName() << " (+" << static_cast<int>(item.getEffectValue()) << ")\n";
        }

        if (activeSlots.empty()) {
            std::cout << "(nothing equipped)\n0. Back\n";
            waitForEnter();
            return;
        }

        std::cout << "0. Back\nSelect slot to unequip: " << std::flush;
        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            discardRestOfLine();
            continue;
        }
        discardRestOfLine();
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(activeSlots.size())) {
            std::cout << "Invalid.\n";
            waitForEnter();
            continue;
        }

        if (!player.hasInventorySpace()) {
            std::cout << "[Inventory] Backpack full — make room first.\n";
            waitForEnter();
            continue;
        }

        if (!player.unequipSlot(activeSlots[static_cast<size_t>(choice - 1)])) {
            std::cout << "[Inventory] Could not unequip.\n";
        }
        waitForEnter();
    }
}

void ConsoleUI::runPreRoomMenu(Player &player, int roomNumber, int totalRooms,
                               const std::string& welcomeBanner) {
    bool showWelcome = !welcomeBanner.empty();
    while (true) {
        clearScreen();
        if (showWelcome) {
            std::cout << welcomeBanner;
            showWelcome = false;
        }
        if (roomNumber > 0 && totalRooms > 0) {
            std::cout << "\n===== Room " << roomNumber << " / " << totalRooms << " =====\n";
        }
        showPlayerStatus(player);

        std::cout << "\n[C] Continue  [I] Inventory  [U] Use/Equip  [E] Unequip\nChoice: "
                  << std::flush;
        discardRestOfLineIfBuffered();
        std::string line;
        if (!std::getline(std::cin, line)) return;

        if (!line.empty() && (line[0] == 'c' || line[0] == 'C')) return;
        if (line.empty()) {
            std::cout << "Press C to enter the room.\n";
            waitForEnter();
            continue;
        }

        if (line.size() == 1 && (line[0] == 'i' || line[0] == 'I')) {
            showInventory(player);
            waitForEnter();
            continue;
        }
        if (line.size() == 1 && (line[0] == 'u' || line[0] == 'U')) {
            runInventoryUseMenu(player);
            continue;
        }
        if (line.size() == 1 && (line[0] == 'e' || line[0] == 'E')) {
            ConsoleUI::runUnequipMenu(player);
            continue;
        }

        std::cout << "Invalid. Press C to enter the room, or I / U / E for inventory.\n";
        waitForEnter();
    }
}
