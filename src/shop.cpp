#include "shop.h"
#include "merchant.h"
#include "player.h"
#include "gamelogger.h"
#include "item.h"
#include <iostream>
#include <algorithm>
#include <limits>

Shop::Shop() {
    isShopOpen = false;
    sellDiscount = 0.5f;
    merchant = nullptr;
    player = nullptr;
    inventory = nullptr;
    purchaseCount = 0;
}

Shop::~Shop() {
    merchant = nullptr;
    player = nullptr;
    inventory = nullptr;
}

void Shop::initShop(Merchant* m, Player* p) {
    merchant = m;
    player = p;
    inventory = p->get_inventory();
    isShopOpen = true;
    purchaseCount = 0;
}

int Shop::calculateSellPrice(const Item& item) {
    return static_cast<int>(item.getPrice() * sellDiscount);
}

bool Shop::buyItem(ItemType type, int grade) {
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }

    if (purchaseCount >= 5) {
    std::cout << "[Shop] You have bought enough items, shop closes now." << std::endl;
    closeShop();
    return false;
    }

    if (!merchant->hasItem(type)) {
        std::cout << "[Shop] Item not available." << std::endl;
        return false;
    }

    int diff = merchant->getCurrentDiff();
    float multiplier = 1.0f;
    if (diff == 0) multiplier = 0.8f;
    else if (diff == 1) multiplier = 1.0f;
    else multiplier = 1.3f;

    Item item = merchant->getItem(type);
    int finalPrice = static_cast<int>(item.getPrice() * multiplier);

    if (player->get_Money() < finalPrice) {
        std::cout << "[Shop] Not enough gold." << std::endl;
        return false;
    }

    if (inventory->get_current_size() >= inventory->get_capacity()) {
        std::cout << "[Shop] Inventory full." << std::endl;
        return false;
    }

    player->change_Money(-finalPrice);
    inventory->add_item(item.getId());

    purchaseCount++;

    // Immediate effects for merchant-room purchases.
    if (type == WEAPON) {
        player->change_ATK(10.0f);
        std::cout << "[Shop] Sword bonus applied: +10 ATK." << std::endl;
    } else if (type == ARMOR) {
        player->change_DEF(10.0f);
        std::cout << "[Shop] Armor bonus applied: +10 DEF." << std::endl;
    } else if (type == POTION) {
        int heal = 10 + (std::rand() % 14);
        player->change_HP(static_cast<float>(heal));
        std::cout << "[Shop] Potion used immediately: +" << heal << " HP." << std::endl;
    }

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("BUY", type, grade, finalPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Bought: " << item.getName() << " for " << finalPrice << " gold." << std::endl;

    std::cout << "\n[Shop] Current Inventory: " 
              << inventory->get_current_size() 
              << " / " 
              << inventory->get_capacity() 
              << std::endl;

    return true;
}

bool Shop::sellItem(ItemType type, int grade) {
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }
    int targetId = -1;
    const auto& items = inventory->get_items();
    for (int id : items) {
        Item owned(id);
        if (owned.getType() != type) continue;
        if (grade >= 0 && grade <= 2 && static_cast<int>(owned.getRarity()) != grade) continue;
        targetId = id;
        break;
    }

    if (targetId < 0) {
        std::cout << "[Shop] You don't have a matching item." << std::endl;
        return false;
    }

    Item item(targetId);
    int sellPrice = calculateSellPrice(item);
    inventory->remove_item(targetId);
    player->change_Money(sellPrice);

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("SELL", type, grade, sellPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Sold: " << item.getName() << " for " << sellPrice << " gold." << std::endl;
    return true;
}

void Shop::showShopUI() {
    if (!merchant || !player || !inventory) return;

    int choice = -1;
    while (isShopOpen) {
        merchant->showGoodsList();
        std::cout << "\n[Shop] Choose an option:\n";
        std::cout << "1. Buy Sword (+10 ATK)\n";
        std::cout << "2. Buy Potion (+10~23 HP)\n";
        std::cout << "3. Buy Armor (+10 DEF)\n";
        std::cout << "0. Leave Shop\n";
        std::cout << "Choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "[Shop] Invalid input.\n";
            continue;
        }

        if (choice == 0) break;
        if (choice == 1) buyItem(WEAPON, static_cast<int>(MEDIUM));
        else if (choice == 2) buyItem(POTION, static_cast<int>(MEDIUM));
        else if (choice == 3) buyItem(ARMOR, static_cast<int>(MEDIUM));
        else std::cout << "[Shop] Invalid option.\n";
    }
}

void Shop::closeShop() {
    isShopOpen = false;
    purchaseCount = 0;
}

bool Shop::get_isShopOpen() const {
    return isShopOpen;
}

float Shop::get_sellDiscount() const {
    return sellDiscount;
}
