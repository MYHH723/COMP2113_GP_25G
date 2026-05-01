#include "merchant.h"
#include "item.h"
#include "types.h"
#include <iostream>

Merchant::Merchant(int gameDiff, int seed) : isAvailable(true), currentDiff(gameDiff) {
    (void)seed;
    initMerchant();
}

Merchant::~Merchant() = default;

void Merchant::initMerchant() {
    restockGoods(POTION);
    restockGoods(WEAPON);
    restockGoods(ARMOR);
}

bool Merchant::hasItem(ItemType type) const {
    return goods.find(type) != goods.end();
}

Item Merchant::getItem(ItemType type) {
    int id = goods[type];
    return Item(id);
}

void Merchant::showGoodsList() const {
    std::cout << "\n===== Merchant Shop =====" << std::endl;
    for (const auto& pair : goods) {
        Item temp(pair.second);
        std::cout << "- " << temp.getName()
                  << " (Price: " << temp.getPrice() << " gold)" << std::endl;
    }
    std::cout << "=========================" << std::endl;
}

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

bool Merchant::getIsAvailable() const { return isAvailable; }
void Merchant::setIsAvailable(bool state) { isAvailable = state; }
int Merchant::getCurrentDiff() const { return currentDiff; }
