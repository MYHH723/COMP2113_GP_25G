#ifndef MERCHANT_H
#define MERCHANT_H

#include "item.h"
#include "types.h"
#include <map>
#include <array>
#include <string>

// Merchant class - manages goods display and stock
class Merchant {
private:
    std::map<ItemType, std::array<int, 3>> goodsByGrade;
    bool isAvailable;
    int currentDiff;
    int nextItemId;

    void restockGoods(ItemType type, ItemRarity rarity);

public:
    Merchant();
    Merchant(int gameDiff, int seed);
    ~Merchant();

    void initMerchant();

    bool hasItem(ItemType type, int grade) const;
    Item getItem(ItemType type, int grade);
    void showGoodsList() const;

    bool getIsAvailable() const;
    void setIsAvailable(bool status);
    int getCurrentDiff() const;
};

const int MAX_STOCK = 999;

/** Rebuild merchant itemDatabase entries deterministically from game seed. */
void seedMerchantCatalog(int gameDiff, int seed);

#endif // MERCHANT_H
