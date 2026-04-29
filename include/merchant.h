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
    std::map<ItemType, int> goods;  // 3 grades per item type
    int maxStockPerItem;
    bool isAvailable;
    int currentDiff;

public:
    Merchant();
    Merchant(int gameDiff,int seed);
    ~Merchant();

    // Initialization
    void initMerchant();

    // Goods management
    bool hasItem(ItemType type) const;
    Item getItem(ItemType type);
    void showGoodsList() const;
    void restockGoods(ItemType type);

    // Availability
    bool getIsAvailable() const;
    void setIsAvailable(bool status);

    // Getters
    int getCurrentDiff() const;
};

// Global constants
const int MAX_STOCK = 999;

#endif // MERCHANT_H