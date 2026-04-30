#include "shop.h"
#include "merchant.h"
#include "player.h"
#include "gamelogger.h"
#include "item.h"
#include <iostream>
#include <algorithm>

Shop::Shop() {
    isShopOpen = false;
    sellDiscount = 0.5f;
    merchant = nullptr;
    player = nullptr;
    inventory = nullptr;
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
}

int Shop::calculateSellPrice(const Item& item) {
    return static_cast<int>(item.getPrice() * sellDiscount);
}

bool Shop::buyItem(ItemType type, int grade) {
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
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

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("BUY", type, grade, finalPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Bought: " << item.getName() << " for " << finalPrice << " gold." << std::endl;
    return true;
}

bool Shop::sellItem(ItemType type, int grade) {
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }
    if (!merchant->hasItem(type)) {
        std::cout << "[Shop] Item not available." << std::endl;
        return false;
    }

    Item item = merchant->getItem(type);
    int itemId = item.getId();

    const auto& items = inventory->get_items();
    auto it = std::find(items.begin(), items.end(), itemId);
    if (it == items.end()) {
        std::cout << "[Shop] You don't have this item." << std::endl;
        return false;
    }

    int sellPrice = calculateSellPrice(item);
    inventory->remove_item(itemId);
    player->change_Money(sellPrice);

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("SELL", type, grade, sellPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Sold: " << item.getName() << " for " << sellPrice << " gold." << std::endl;
    return true;
}

void Shop::showShopUI() {
    if (merchant) merchant->showGoodsList();
}

void Shop::closeShop() {
    isShopOpen = false;
}

bool Shop::get_isShopOpen() const {
    return isShopOpen;
}

float Shop::get_sellDiscount() const {
    return sellDiscount;
}
