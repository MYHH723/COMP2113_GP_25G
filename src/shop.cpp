#include "shop.h"
#include "merchant.h"
#include "player.h"
#include "item.h"
#include "utils.h"
#include "consoleUI.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace {

// Get static random number generator for merchant dialogue
std::mt19937& merchantRng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}

// Print random merchant dialogue line
void sayMerchant(const char* const* lines, size_t count) {
    if (count == 0) return;
    std::uniform_int_distribution<size_t> pick(0, count - 1);
    std::cout << "  Merchant: \"" << lines[pick(merchantRng())] << "\"\n";
}

// Merchant dialogue lines
const char* const kWelcome[] = {
    "Ah, traveller - your coin sings sweeter than chapel bells in this dreary hall.",
    "Step closer, wanderer. Steel, salve, and honest steel await the bold.",
    "Gold opens every lock - even those the dungeon keeps.",
    "Welcome to my stall; what you lack, I may yet sell.",
    "The road is cruel; let gear be kinder than fate.",
};

const char* const kFarewell[] = {
    "Fair fortune on the road, stranger - may your purse stay heavy.",
    "Till coin calls again: keep blade sharp and faith sharper.",
    "Go with steel bright and heart undimmed.",
    "The door stays open when gold remembers the way back.",
    "May your next hall hold treasure, not tomb.",
};

const char* const kBuyWeapon[] = {
    "A worthy blade - may it carve your legend in the dark.",
    "Iron honest and true; swear it only to honour.",
    "This steel has thirst - let it drink deep of shadow.",
};

const char* const kBuyArmor[] = {
    "Plate and mail - let them bear what flesh cannot.",
    "Good mail turns death aside; wear it like a vow.",
    "The forge remembers oaths; this harness keeps them.",
};

const char* const kBuyPotion[] = {
    "Drink deep - the brew remembers old remedies and older prayers.",
    "A draught for the weary; let crimson stay within your veins.",
    "The apothecary's art: bitter sip, sweeter dawn.",
};

const char* const kTooPoor[] = {
    "Nay - the purse speaks louder than courage today.",
    "Thin coin, thin mercy - return when gold outweighs pride.",
    "Alas, my friend, dreams weigh less than scales.",
};

const char* const kInventoryFull[] = {
    "Your pack groans full; lighten it ere you buy more.",
    "Even merchants marvel - where do you hide so much?",
    "No space for breath, let alone brass - make room first.",
};

const char* const kNoStock[] = {
    "That shelf stands bare - another soul claimed it first.",
    "Would that I had it to sell; come again anon.",
};

const char* const kSellToMerchant[] = {
    "Fair trade - your cast-offs become another's fortune.",
    "Gold changes hands; honour keeps its tally.",
    "I'll find a buyer; may this coin warm your road.",
};

const char* const kNoMatchItem[] = {
    "I see no such ware upon your person - bring what matches.",
    "That bauble you lack; search your pack again.",
};

} // namespace

// Default constructor
Shop::Shop() {
    isShopOpen = false;
    sellDiscount = 0.5f;
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

// Initialize shop with merchant and player references
void Shop::initShop(Merchant* m, Player* p) {
    merchant = m;
    player = p;
    inventory = p->get_inventory();
    isShopOpen = true;
}

// Calculate sell price for item (50% of original price)
int Shop::calculateSellPrice(const Item& item) {
    return static_cast<int>(item.getPrice() * sellDiscount);
}

// Buy item from merchant
bool Shop::buyItem(ItemType type, int grade) {
    (void)grade;
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }
    if (!merchant->hasItem(type)) {
        sayMerchant(kNoStock, sizeof(kNoStock) / sizeof(kNoStock[0]));
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
        sayMerchant(kTooPoor, sizeof(kTooPoor) / sizeof(kTooPoor[0]));
        std::cout << "[Shop] Not enough gold." << std::endl;
        return false;
    }

    if (inventory->get_current_size() >= inventory->get_capacity()) {
        sayMerchant(kInventoryFull, sizeof(kInventoryFull) / sizeof(kInventoryFull[0]));
        std::cout << "[Shop] Inventory full." << std::endl;
        return false;
    }

    player->change_Money(-finalPrice);
    inventory->add_item(item.getId());

    // Apply immediate effects for purchased items
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

    // Play corresponding dialogue
    if (type == WEAPON) {
        sayMerchant(kBuyWeapon, sizeof(kBuyWeapon) / sizeof(kBuyWeapon[0]));
    } else if (type == ARMOR) {
        sayMerchant(kBuyArmor, sizeof(kBuyArmor) / sizeof(kBuyArmor[0]));
    } else if (type == POTION) {
        sayMerchant(kBuyPotion, sizeof(kBuyPotion) / sizeof(kBuyPotion[0]));
    }

    std::cout << "[Shop] Bought: " << item.getName() << " for " << finalPrice << " gold." << std::endl;
    return true;
}

// Sell item to merchant
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
        sayMerchant(kNoMatchItem, sizeof(kNoMatchItem) / sizeof(kNoMatchItem[0]));
        std::cout << "[Shop] You don't have a matching item." << std::endl;
        return false;
    }

    Item item(targetId);
    int sellPrice = calculateSellPrice(item);
    inventory->remove_item(targetId);
    player->change_Money(sellPrice);

    sayMerchant(kSellToMerchant, sizeof(kSellToMerchant) / sizeof(kSellToMerchant[0]));
    std::cout << "[Shop] Sold: " << item.getName() << " for " << sellPrice << " gold." << std::endl;
    return true;
}

// Show main shop UI and handle user input
void Shop::showShopUI() {
    if (!merchant || !player || !inventory) return;

    sayMerchant(kWelcome, sizeof(kWelcome) / sizeof(kWelcome[0]));

    int choice = -1;
    while (true) {
        merchant->showGoodsList();
        std::cout << "\n[Shop] Choose an option:\n";
        std::cout << "1. Upgrade Sword (+10 ATK)\n";
        std::cout << "2. Buy Potion (+10~23 HP)\n";
        std::cout << "3. Upgrade Armor (+10 DEF)\n";
        std::cout << "0. Leave Shop\n";
        std::cout << "Choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "[Shop] Invalid input. Enter a number (0-3).\n";
            continue;
        }
        discardRestOfLine();

        if (choice == 0) {
            sayMerchant(kFarewell, sizeof(kFarewell) / sizeof(kFarewell[0]));
            break;
        }

        auto refreshStatusAfterBuy = [&]() {
            clearScreen();
            ConsoleUI::showPlayerStatus(*player);
        };

        if (choice == 1) {
            if (buyItem(WEAPON, static_cast<int>(MEDIUM))) refreshStatusAfterBuy();
        } else if (choice == 2) {
            if (buyItem(POTION, static_cast<int>(MEDIUM))) refreshStatusAfterBuy();
        } else if (choice == 3) {
            if (buyItem(ARMOR, static_cast<int>(MEDIUM))) refreshStatusAfterBuy();
        } else {
            std::cout << "[Shop] Invalid option.\n";
        }
    }
}

// Close the shop
void Shop::closeShop() {
    isShopOpen = false;
}

// Check if shop is open
bool Shop::get_isShopOpen() const {
    return isShopOpen;
}

// Get current sell discount rate
float Shop::get_sellDiscount() const {
    return sellDiscount;
}
