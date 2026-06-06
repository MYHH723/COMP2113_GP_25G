#include "player.h"
#include "shop.h"
#include "merchant.h"
#include "item.h"
#include "utils.h"
#include "consoleUI.h"
#include <iostream>
#include <algorithm>

namespace {

// Print random merchant dialogue line
void sayMerchant(const char* const* lines, size_t count) {
    if (count == 0) return;
    const size_t idx = static_cast<size_t>(getRandom(0, static_cast<int>(count) - 1));
    std::cout << "  Merchant: \"" << lines[idx] << "\"\n";
}

// Merchant dialogue lines (medieval flavour; one line chosen at random)
const char* const kWelcome[] = {
    "Ah, traveller - your coin sings sweeter than chapel bells in this dreary hall.",
    "Step closer, wanderer. Steel, salve, and honest steel await the bold.",
    "Gold opens every lock - even those the dungeon keeps.",
    "Welcome to my stall; what you lack, I may yet sell.",
    "The road is cruel; let gear be kinder than fate.",
    "By Saint Cuthbert's scales - browse, but do not bleed upon my wares.",
    "A pilgrim of the deep halls? Good. My prices favour the living.",
    "The torchlight flatters steel and villain alike; choose what keeps you whole.",
    "I trade with knights, thieves, and shades - so long as the coin is true.",
    "Rest your feet, empty your purse, and may the vaults be kinder than the road.",
};

const char* const kFarewell[] = {
    "Fair fortune on the road, stranger - may your purse stay heavy.",
    "Till coin calls again: keep blade sharp and faith sharper.",
    "Go with steel bright and heart undimmed.",
    "The door stays open when gold remembers the way back.",
    "May your next hall hold treasure, not tomb.",
    "Walk in light, if light there be - these stones remember every footfall.",
    "Farewell. Tell the reaper I am not yet ready to close my ledger.",
    "God speed, and may your mail turn aside what wit cannot.",
    "The dungeon hears oaths it never keeps - keep yours nonetheless.",
    "Until we meet again: spend not your last groat on pride.",
};

const char* const kBuyWeapon[] = {
    "A worthy blade - may it carve your legend in the dark.",
    "Iron honest and true; swear it only to honour.",
    "This steel has thirst - let it drink deep of shadow.",
    "A sword is a sermon written in metal; preach wisely.",
    "The smith's fire blessed this edge - see that it does not dull in shame.",
    "Take it. Let foes learn the cost of standing in your path.",
    "From forge to fist - may your grip be steadier than your fear.",
    "A knight without steel is a psalm without breath.",
};

const char* const kBuyArmor[] = {
    "Plate and mail - let them bear what flesh cannot.",
    "Good mail turns death aside; wear it like a vow.",
    "The forge remembers oaths; this harness keeps them.",
    "Let rivets hold where courage wavers.",
    "Armour is the second skin God denied mortals - wear it humbly.",
    "May every dent become a tale you survive to tell.",
    "This hauberk has turned spears ere now; it may turn yours as well.",
    "Dress for judgement; the dungeon shows little mercy to the bare.",
};

const char* const kBuyPotion[] = {
    "Drink deep - the brew remembers old remedies and older prayers.",
    "A draught for the weary; let crimson stay within your veins.",
    "The apothecary's art: bitter sip, sweeter dawn.",
    "Herbs gathered by moonlight - or so the monks swore.",
    "One swallow between you and the grave; do not savour it overlong.",
    "The flask hums with alchemy; trust it more than despair.",
    "A cordial for battered sinew and bruised spirit alike.",
    "Bitter as penance, yet kinder than the undertaker's cart.",
};

const char* const kTooPoor[] = {
    "Nay - the purse speaks louder than courage today.",
    "Thin coin, thin mercy - return when gold outweighs pride.",
    "Alas, my friend, dreams weigh less than scales.",
    "Even a miserly abbot would pity that purse.",
    "My goods are not charity, nor am I a friar.",
    "Come again when your coffers clink like chapel plate.",
    "The dungeon takes freely; I, alas, must charge.",
    "Empty hands buy empty promises - fill them first.",
};

const char* const kInventoryFull[] = {
    "Your pack groans full; lighten it ere you buy more.",
    "Even merchants marvel - where do you hide so much?",
    "No space for breath, let alone brass - make room first.",
    "You carry a war camp on your back; shed some plunder.",
    "My stall is wide, your satchel is not - make choice.",
    "A hoarder's heart fills faster than a beggar's sack.",
    "Sell or discard, friend - greed weighs more than iron.",
    "No room for another buckle, unless you lose a burden.",
};

const char* const kNoStock[] = {
    "That shelf stands bare - another soul claimed it first.",
    "Would that I had it to sell; come again anon.",
    "Sold to the last pilgrim; fortune favours the swift.",
    "Gone, like honour in a thieves' guild.",
    "The vault behind me is empty of that ware.",
    "Another bought your prize while you dallied in the dark.",
    "Restock comes with dawn - if dawn ever finds this place.",
    "Naught left but dust and the memory of better days.",
};

const char* const kSellToMerchant[] = {
    "Fair trade - your cast-offs become another's fortune.",
    "Gold changes hands; honour keeps its tally.",
    "I'll find a buyer; may this coin warm your road.",
    "A sensible bargain - I have seen worse in the king's market.",
    "Your loss is my ledger's gain; such is commerce.",
    "Taken. May it serve some soul less clumsy than its last master.",
    "The scales balance; neither saint nor devil argues with copper.",
    "Sold. Spend the proceeds before the dungeon claims them back.",
};

const char* const kDualWieldNoArmor[] = {
    "Two blades demand both hands - mail must wait in your pack.",
    "Twin steel leaves no room for plate; stow the harness if you dare.",
    "A second sword, a second oath - armour cannot share the grip.",
    "Dual blades sing; let your hauberk rest until one is sheathed.",
};

const char* const kTripleArmorNoDual[] = {
    "Three layers of mail - both hands are needed to bear the weight.",
    "Plate upon plate; a second blade would topple you like a siege tower.",
    "Full harness leaves no hand free for twin steel.",
    "You are clad thrice over - one sword must suffice.",
};

const char* const kTripleArmorActive[] = {
    "Mail stacked thrice - walk proud, but keep one blade only.",
    "Threefold harness; let a single sword guard the gaps.",
    "Layered plate turns spears aside - dual blades would bind your arms.",
};

const char* const kArmorEndsDualWield[] = {
    "One blade must rest before mail can be donned again.",
    "Plate returns - your off-hand steel sleeps in the satchel.",
    "Armour claims the body; twin swords cannot both stay drawn.",
};

const char* const kNoMatchItem[] = {
    "I see no such ware upon your person - bring what matches.",
    "That bauble you lack; search your pack again.",
    "My eyes are old, yet not so old they miss an empty hand.",
    "You ask for steel I cannot see - check belt and bundle.",
    "No matching goods. Perhaps you sold it in a fever dream.",
    "Bring me the article itself, not its ghost.",
    "Wrong type, wrong grade, or wrong traveller - try again.",
    "I trade in truth, not wishes; produce the item.",
};

int promptGrade() {
    while (true) {
        std::cout << "Grade: 0=Back  1=Low  2=Medium  3=High: ";
        int gradeChoice = 0;
        if (!(std::cin >> gradeChoice)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "[Shop] Invalid input.\n";
            continue;
        }
        discardRestOfLine();
        if (gradeChoice == 0) return -1;
        if (gradeChoice >= 1 && gradeChoice <= 3) {
            return gradeChoice - 1;
        }
        std::cout << "[Shop] Enter 0 to go back, or 1, 2, or 3.\n";
    }
}

} // namespace

void Shop::logTransaction(const std::string& action, ItemType type, int grade, int gold) {
    if (!transactionLogActive) return;
    transactionLogger.logTransaction(action, type, grade, gold);
}

// Default constructor
Shop::Shop()
    : merchant(nullptr),
      player(nullptr),
      inventory(nullptr),
      isShopOpen(false),
      sellDiscount(0.5f),
      transactionLogActive(false) {
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
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }
    if (grade < 0) return false;
    if (grade > 2) grade = static_cast<int>(MEDIUM);
    if (!merchant->hasItem(type, grade)) {
        sayMerchant(kNoStock, sizeof(kNoStock) / sizeof(kNoStock[0]));
        std::cout << "[Shop] Item not available." << std::endl;
        return false;
    }

    int diff = merchant->getCurrentDiff();
    float multiplier = 1.0f;
    if (diff == 0) multiplier = 0.8f;
    else if (diff == 1) multiplier = 1.0f;
    else multiplier = 1.3f;

    Item item = merchant->getItem(type, grade);
    int finalPrice = static_cast<int>(item.getPrice() * multiplier);

    if (player->get_Money() < finalPrice) {
        sayMerchant(kTooPoor, sizeof(kTooPoor) / sizeof(kTooPoor[0]));
        std::cout << "[Shop] Not enough gold." << std::endl;
        return false;
    }

    if (!player->hasInventorySpace()) {
        sayMerchant(kInventoryFull, sizeof(kInventoryFull) / sizeof(kInventoryFull[0]));
        std::cout << "[Shop] Inventory full." << std::endl;
        return false;
    }

    player->change_Money(-finalPrice);
    if (!inventory->add_item(item.getId())) {
        player->change_Money(finalPrice);
        sayMerchant(kInventoryFull, sizeof(kInventoryFull) / sizeof(kInventoryFull[0]));
        std::cout << "[Shop] Inventory full." << std::endl;
        return false;
    }

    const int ownedCount = player->countOwnedItems(item.getId());
    std::cout << "[Shop] Bought: " << item.getName() << " for " << finalPrice << " gold.\n";
    std::cout << "[Shop] Added to backpack. You own " << ownedCount << "x " << item.getName() << ".\n";

    if (type == POTION) {
        if (promptYesNo("Use potion now? (y/n): ")) {
            if (player->usePotionFromInventory(item.getId())) {
                std::cout << "[Shop] Potion consumed.\n";
            } else {
                std::cout << "[Shop] Could not use potion.\n";
            }
        } else {
            std::cout << "[Shop] Potion kept in backpack.\n";
        }
    } else {
        const bool wasDualWield = player->isDualWielding();
        if (promptYesNo("Equip now? (y/n): ")) {
            if (!player->equipFromInventory(item.getId())) {
                std::cout << "[Shop] Could not equip (check dual-wield / armor rules or backpack space).\n";
            } else {
                if (type == WEAPON) {
                    if (player->isDualWielding()) {
                        sayMerchant(kDualWieldNoArmor, sizeof(kDualWieldNoArmor) / sizeof(kDualWieldNoArmor[0]));
                        std::cout << "[Shop] Dual-wield active (armor disabled).\n";
                    }
                    std::cout << "[Shop] Sword equipped: +"
                              << static_cast<int>(item.getEffectValue()) << " ATK.\n";
                } else if (type == ARMOR) {
                    if (wasDualWield) {
                        sayMerchant(kArmorEndsDualWield, sizeof(kArmorEndsDualWield) / sizeof(kArmorEndsDualWield[0]));
                    }
                    if (player->isFullLayeredArmor()) {
                        sayMerchant(kTripleArmorActive, sizeof(kTripleArmorActive) / sizeof(kTripleArmorActive[0]));
                        std::cout << "[Shop] Triple armor active (single sword only).\n";
                    }
                    std::cout << "[Shop] Armor equipped: +"
                              << static_cast<int>(item.getEffectValue()) << " DEF.\n";
                }
            }
        } else {
            std::cout << "[Shop] Item kept in backpack.\n";
        }
    }

    // Play corresponding dialogue
    if (type == WEAPON) {
        sayMerchant(kBuyWeapon, sizeof(kBuyWeapon) / sizeof(kBuyWeapon[0]));
    } else if (type == ARMOR) {
        sayMerchant(kBuyArmor, sizeof(kBuyArmor) / sizeof(kBuyArmor[0]));
    } else if (type == POTION) {
        sayMerchant(kBuyPotion, sizeof(kBuyPotion) / sizeof(kBuyPotion[0]));
    }

    logTransaction("BUY", type, grade, finalPrice);
    return true;
}

// Sell item to merchant
bool Shop::sellItem(ItemType type, int grade) {
    if (!isShopOpen) {
        std::cout << "[Shop] Shop is closed." << std::endl;
        return false;
    }
    if (grade < 0) return false;

    auto matchesGrade = [&](const Item& owned) {
        return grade < 0 || grade > 2 || static_cast<int>(owned.getRarity()) == grade;
    };

    int targetId = -1;
    const char* targetSlot = nullptr;

    if (type == WEAPON) {
        for (const char* slot : {SLOT_WEAPON, SLOT_WEAPON2}) {
            const int equippedId = player->getEquippedId(slot);
            if (equippedId == 0) continue;
            Item equipped(equippedId);
            if (equipped.getType() == WEAPON && matchesGrade(equipped)) {
                targetId = equippedId;
                targetSlot = slot;
                break;
            }
        }
    } else if (type == ARMOR) {
        for (const char* slot : {SLOT_ARMOR, SLOT_ARMOR2, SLOT_ARMOR3}) {
            const int equippedId = player->getEquippedId(slot);
            if (equippedId == 0) continue;
            Item equipped(equippedId);
            if (equipped.getType() == ARMOR && matchesGrade(equipped)) {
                targetId = equippedId;
                targetSlot = slot;
                break;
            }
        }
    }

    if (targetId < 0) {
        const auto& items = inventory->get_items();
        for (int id : items) {
            Item owned(id);
            if (owned.getType() != type) continue;
            if (!matchesGrade(owned)) continue;
            targetId = id;
            break;
        }
    }

    if (targetId < 0) {
        sayMerchant(kNoMatchItem, sizeof(kNoMatchItem) / sizeof(kNoMatchItem[0]));
        std::cout << "[Shop] You don't have a matching item." << std::endl;
        return false;
    }

    Item item(targetId);
    const int sellPrice = calculateSellPrice(item);

    if (targetSlot != nullptr) {
        player->unequipSlot(targetSlot, false);
    } else {
        inventory->remove_item(targetId);
    }
    player->change_Money(sellPrice);

    sayMerchant(kSellToMerchant, sizeof(kSellToMerchant) / sizeof(kSellToMerchant[0]));
    std::cout << "[Shop] Sold: " << item.getName() << " for " << sellPrice << " gold." << std::endl;
    logTransaction("SELL", type, grade, sellPrice);
    return true;
}

// Show main shop UI and handle user input
void Shop::showShopUI() {
    if (!merchant || !player || !inventory) return;

    transactionLogger.initLogFile();
    transactionLogActive = true;

    sayMerchant(kWelcome, sizeof(kWelcome) / sizeof(kWelcome[0]));

    int choice = -1;
    while (true) {
        merchant->showGoodsList();
        std::cout << "\n[Shop] Choose an option:\n";
        std::cout << "1. Buy Sword (pick grade; optional equip after purchase)\n";
        std::cout << "2. Buy Potion (pick grade; optional use after purchase)\n";
        std::cout << "3. Buy Armor (up to 3 layers; optional equip after purchase)\n";
        std::cout << "4. Sell item (pick type and grade)\n";
        std::cout << "5. Unequip worn gear to backpack\n";
        std::cout << "0. Leave Shop\n";
        std::cout << "Choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            discardRestOfLine();
            std::cout << "[Shop] Invalid input. Enter a number (0-5).\n";
            continue;
        }
        discardRestOfLine();

        if (choice == 0) {
            sayMerchant(kFarewell, sizeof(kFarewell) / sizeof(kFarewell[0]));
            transactionLogActive = false;
            break;
        }

        auto refreshStatusAfterBuy = [&]() {
            clearScreen();
            ConsoleUI::showPlayerStatus(*player);
        };

        if (choice == 1) {
            const int grade = promptGrade();
            if (grade >= 0 && buyItem(WEAPON, grade)) refreshStatusAfterBuy();
        } else if (choice == 2) {
            const int grade = promptGrade();
            if (grade >= 0 && buyItem(POTION, grade)) refreshStatusAfterBuy();
        } else if (choice == 3) {
            const int grade = promptGrade();
            if (grade >= 0 && buyItem(ARMOR, grade)) refreshStatusAfterBuy();
        } else if (choice == 4) {
            std::cout << "Sell type: 0=Back  1=Sword  2=Potion  3=Armor: ";
            int sellType = 0;
            if (!(std::cin >> sellType)) {
                std::cin.clear();
                discardRestOfLine();
                std::cout << "[Shop] Invalid input.\n";
                continue;
            }
            discardRestOfLine();
            if (sellType == 0) continue;
            ItemType type = POTION;
            if (sellType == 1) type = WEAPON;
            else if (sellType == 2) type = POTION;
            else if (sellType == 3) type = ARMOR;
            else {
                std::cout << "[Shop] Invalid sell type.\n";
                continue;
            }
            const int grade = promptGrade();
            if (grade >= 0 && sellItem(type, grade)) refreshStatusAfterBuy();
        } else if (choice == 5) {
            ConsoleUI::runUnequipMenu(*player);
            refreshStatusAfterBuy();
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
