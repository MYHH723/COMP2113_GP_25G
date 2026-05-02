#include "merchant.h"
#include "item.h"
#include "types.h"
#include <iostream>
#include <random>

namespace {

// Merchant dialogue lines for shop interaction flavor text
const char* const kHawk[] = {
    "Fresh ware from caravan and crypt alike!",
    "Prices fair as moonlight - feel the weight of each piece.",
    "Names mean little; coin names everything.",
    "See here - steel that remembers honour, draught that remembers life.",
};

// Print random merchant dialogue line
void mutterMerchantLine() {
    static std::mt19937 g(std::random_device{}());
    std::uniform_int_distribution<size_t> pick(0, sizeof(kHawk) / sizeof(kHawk[0]) - 1);
    std::cout << "  Merchant: \"" << kHawk[pick(g)] << "\"\n";
}

} // namespace

// Constructor: Initialize merchant with game difficulty and seed
Merchant::Merchant(int gameDiff, int seed) : isAvailable(true), currentDiff(gameDiff) {
    (void)seed;
    initMerchant();
}

// Default destructor
Merchant::~Merchant() = default;

// Initialize merchant inventory
void Merchant::initMerchant() {
    restockGoods(POTION);
    restockGoods(WEAPON);
    restockGoods(ARMOR);
}

// Check if merchant has specified item type in stock
bool Merchant::hasItem(ItemType type) const {
    return goods.find(type) != goods.end();
}

// Get item of specified type from merchant inventory
Item Merchant::getItem(ItemType type) {
    int id = goods[type];
    return Item(id);
}

// Display all available goods in merchant shop
void Merchant::showGoodsList() const {
    mutterMerchantLine();
    std::cout << "\n===== Merchant Shop =====" << std::endl;
    for (const auto& pair : goods) {
        Item temp(pair.second);
        std::cout << "- " << temp.getName()
                  << " (Price: " << temp.getPrice() << " gold)" << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

// Restock specified item type in merchant inventory
void Merchant::restockGoods(ItemType type) {
    static int nextId = 10000;
    int id = nextId++;
    std::string name;
    int cost = 0;
    float effectValue = 0;
    ItemRarity rarity = LOW;

    if (type == POTION) {
        name = "Potion";
        effectValue = 16;
        cost = 20;
        rarity = MEDIUM;
    } else if (type == WEAPON) {
        name = "Sword";
        effectValue = 10;
        cost = 25;
        rarity = MEDIUM;
    } else if (type == ARMOR) {
        name = "Armor";
        effectValue = 10;
        cost = 22;
        rarity = MEDIUM;
    }

    itemDatabase[id] = {name, type, rarity, effectValue, cost, false};
    goods[type] = id;
}

// Check if merchant is available for interaction
bool Merchant::getIsAvailable() const { return isAvailable; }

// Set merchant availability state
void Merchant::setIsAvailable(bool state) { isAvailable = state; }

// Get current game difficulty for merchant
int Merchant::getCurrentDiff() const { return currentDiff; }
