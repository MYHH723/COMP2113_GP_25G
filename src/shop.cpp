#include "shop.h"
#include "merchant.h"
#include "player.h"
#include "gamelogger.h"
#include "item.h"
#include <iostream>
#include <string>

// Constructor
Shop::Shop() {
    isShopOpen = false;
    sellDiscount = SELL_DISCOUNT;
    merchant = nullptr;
    player = nullptr;
    inventory = nullptr;
}

// Destructor
Shop::~Shop() {
    merchant = nullptr;
    player = nullptr;
    inventory = nullptr;
}

// Initialize shop with merchant, player, inventory
void Shop::initShop(Merchant* m, Player* p) {
    this->merchant = m;
    this->player = p;
    this->inventory = p->get_inventory();
    isShopOpen = true;
}

// Calculate selling price with discount
int Shop::calculateSellPrice(const Item& item) {
    int basePrice = item.getPrice();
    return static_cast<int>(basePrice * sellDiscount);
}

// Buy item from merchant (with difficulty-based price + logger + inventory check)
bool Shop::buyItem(ItemType type, int grade) {
    if (!isShopOpen || !merchant || !player || !inventory) {
        std::cout << "[Shop] Shop not initialized or closed." << std::endl;
        return false;
    }

    if (grade < 0 || grade > 2) {
        std::cout << "[Shop] Invalid grade." << std::endl;
        return false;
    }

    if (!merchant->hasItem(type)) {
        std::cout << "[Shop] Item not available." << std::endl;
        return false;
    }

    int diff = merchant->getCurrentDiff();
    float priceMultiplier = 1.0f;

    if (diff == 0) priceMultiplier = 0.8f;
    if (diff == 1) priceMultiplier = 1.0f;
    if (diff == 2) priceMultiplier = 1.3f;

    Item item = merchant->getItem(type);
    int basePrice = item.getPrice();
    int finalPrice = static_cast<int>(basePrice * priceMultiplier);

    if (player->get_Money() < finalPrice) {
        std::cout << "[Shop] Not enough gold." << std::endl;
        return false;
    }

    if (inventory->get_current_size() >= inventory->get_capacity()) {
        std::cout << "[Shop] Inventory is full." << std::endl;
        return false;
    }

    player->change_Money(-finalPrice);
<<<<<<< HEAD
    int id = item.getId();
    inventory->add_item(id);
=======
    std::string itemName = item.getName();
    inventory->add_item(itemName);
>>>>>>> 31434eff26c9f3e059ac5828c7c4afb7c6b30f69

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("BUY", type, grade, finalPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Buy success: " << item.getName() << " Cost: " << finalPrice << " gold" << std::endl;
    return true;
}

// Sell item to merchant (with difficulty-based sell price + logger)
bool Shop::sellItem(ItemType type, int grade) {
    if (!isShopOpen || !merchant || !player || !inventory) {
        std::cout << "[Shop] Shop not initialized or closed." << std::endl;
        return false;
    }

    if (grade < 0 || grade > 2) {
        std::cout << "[Shop] Invalid grade." << std::endl;
        return false;
    }

    Item item = merchant->getItem(type);
    int itemId = item.getId();

    bool hasItem = false;
    for (const int& id : inventory->get_items()) {
        if (id == itemId) {
            hasItem = true;
            break;
        }
    }

    if (!hasItem) {
        std::cout << "[Shop] Item not in inventory." << std::endl;
        return false;
    }

    int diff = merchant->getCurrentDiff();
    float sellMulti = 0.5f;

    if (diff == 0) sellMulti = 0.4f;
    if (diff == 1) sellMulti = 0.5f;
    if (diff == 2) sellMulti = 0.6f;

    int sellPrice = static_cast<int>(item.getPrice() * sellMulti);

<<<<<<< HEAD
    inventory->remove_item(itemId);
=======
    inventory->remove_item(itemName);
>>>>>>> 31434eff26c9f3e059ac5828c7c4afb7c6b30f69
    player->change_Money(sellPrice);

    GameLogger logger;
    logger.initLogFile();
    logger.logTransaction("SELL", type, grade, sellPrice);
    logger.closeLogFile();

    std::cout << "[Shop] Sell success: " << item.getName() << " Earned: " << sellPrice << " gold" << std::endl;
    return true;
}

// Display shop interface
void Shop::showShopUI() {
    if (!merchant) {
        std::cout << "Shop unavailable." << std::endl;
        return;
    }
    merchant->showGoodsList();
}

// Close shop
void Shop::closeShop() {
    isShopOpen = false;
}

// Getters
bool Shop::get_isShopOpen() const {
    return isShopOpen;
}

float Shop::get_sellDiscount() const {
    return sellDiscount;
}
