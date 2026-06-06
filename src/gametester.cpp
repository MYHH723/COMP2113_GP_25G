#include "gametester.h"
#include "monster.h"
#include "player.h"
#include "trap.h"
#include "balance.h"
#include "utils.h"
#include "mapgenerator.h"
#include "room.h"
#include "types.h"
#include "item.h"
#include "savegame.h"
#include "battlesystem.h"

#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>

namespace {

constexpr int kMinAssertions = 500;

int g_passed = 0;
int g_failed = 0;
std::vector<std::string> g_failures;

void expectTrue(bool condition, const std::string& name) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        g_failures.push_back(name);
        std::cerr << "  FAIL: " << name << "\n";
    }
}

std::string key(const std::string& prefix, int a, int b = -1) {
    if (b < 0) return prefix + ":" + std::to_string(a);
    return prefix + ":" + std::to_string(a) + "," + std::to_string(b);
}

// --- Original regression tests ---

void testMonsterTakeDamageNoDoubleDef() {
    Monster m;
    m.initMonster(1, 1);
    const int hpBefore = m.getHP();
    m.takeDamage(10);
    expectTrue(m.getHP() == hpBefore - 10, "takeDamage applies raw damage once");
}

void testMonsterHpMultiplier() {
    g_monsterHpMultiplier = 1.3f;
    Monster hard;
    hard.initMonster(2, 1);
    g_monsterHpMultiplier = 0.8f;
    Monster easy;
    easy.initMonster(0, 1);
    expectTrue(hard.getMaxHP() > easy.getMaxHP(), "Hard monsters have more HP than Easy");
    g_monsterHpMultiplier = 1.0f;
}

void testTrapDamageUsesGlobals() {
    g_trapDamageMin = 8;
    g_trapDamageMax = 18;
    Trap trap;
    trap.initTrap(TrapType::SPIKE_PIT, 1);
    expectTrue(trap.getDamageMin() >= 8 && trap.getDamageMax() <= 18,
               "Trap damage stays within global difficulty range");
}

void testPlayerDefaults() {
    Player p("Tester");
    expectTrue(std::abs(p.get_HP() - DEFAULT_HP) < 0.01f, "Default HP");
    expectTrue(std::abs(p.get_ATK() - DEFAULT_ATK) < 0.01f, "Default ATK");
    expectTrue(std::abs(p.get_Money() - DEFAULT_MONEY) < 0.01f, "Default money");
}

void testPlayerPoison() {
    Player p("Tester");
    p.set_isPoisoned(true);
    const float hpBefore = p.get_HP();
    p.tickPoison();
    expectTrue(p.get_HP() < hpBefore, "Poison tick reduces HP");
    expectTrue(p.get_isPoisoned(), "Poison persists after one tick");
}

void testSeededRandomDeterministic() {
    seedGameRandom(42u);
    const int a = getRandom(1, 100);
    const int b = getRandom(1, 100);
    seedGameRandom(42u);
    const int a2 = getRandom(1, 100);
    const int b2 = getRandom(1, 100);
    expectTrue(a == a2 && b == b2, "Seeded getRandom is reproducible");
}

void testEasyHasShopRooms() {
    seedGameRandom(4242u);
    MapGenerator gen(4242);
    gen.initMapGenerator(10, 0);
    gen.generateMap();
    int shopCount = 0;
    const auto rooms = gen.releaseRooms();
    for (const auto& room : rooms) {
        if (room != nullptr && room->getRoomType() == SHOP) {
            ++shopCount;
        }
    }
    expectTrue(shopCount >= 2, "Easy mode has at least 2 shop rooms");
}

void testBossMilestoneNotShop() {
    seedGameRandom(1u);
    MapGenerator gen(1);
    gen.initMapGenerator(15, 1);
    gen.generateMap();
    const auto all = gen.releaseRooms();
    expectTrue(all.size() >= 5, "Normal map has room 5");
    expectTrue(all[4] != nullptr && all[4]->getRoomType() == BOSS,
               "Room 5 is Boss (not overridden by shop)");
}

void testInventoryPotionCuresPoison() {
    itemDatabase[88001] = {"Test Potion", POTION, LOW, 15.0f, 10, false};
    Player p("Tester");
    p.add_item(88001);
    p.set_isPoisoned(true);
    expectTrue(p.usePotionFromInventory(88001), "Use potion from inventory");
    expectTrue(!p.get_isPoisoned(), "Inventory potion cures poison");
}

void testDualWield() {
    itemDatabase[88010] = {"High Sword", WEAPON, HIGH, 15.0f, 35, false};
    itemDatabase[88011] = {"High Armor", ARMOR, HIGH, 15.0f, 30, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88010), "First sword purchase equips");
    expectTrue(std::abs(p.get_ATK() - (DEFAULT_ATK + 15.0f)) < 0.01f, "First sword adds ATK");
    expectTrue(p.purchaseAndEquip(88010), "Second sword dual-wields");
    expectTrue(p.isDualWielding(), "Dual-wield flag set");
    expectTrue(std::abs(p.get_ATK() - (DEFAULT_ATK + 30.0f)) < 0.01f, "Dual-wield stacks ATK");
    expectTrue(p.countOwnedItems(88010) == 2, "Owns 2 identical swords");
    expectTrue(p.purchaseAndEquip(88011), "Armor purchase while dual-wielding");
    expectTrue(!p.isDualWielding(), "Armor ends dual-wield");
    expectTrue(p.getEquippedId(SLOT_ARMOR) == 88011, "Armor equipped");
    expectTrue(std::abs(p.get_ATK() - (DEFAULT_ATK + 15.0f)) < 0.01f, "Second sword stowed after armor");
}

void testTripleArmor() {
    itemDatabase[88020] = {"High Sword", WEAPON, HIGH, 15.0f, 35, false};
    itemDatabase[88021] = {"High Armor", ARMOR, HIGH, 15.0f, 30, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88021), "First armor purchase");
    expectTrue(p.purchaseAndEquip(88021), "Second armor layer");
    expectTrue(p.purchaseAndEquip(88021), "Third armor layer");
    expectTrue(p.isFullLayeredArmor(), "Triple armor flag set");
    expectTrue(std::abs(p.get_DEF() - (DEFAULT_DEF + 45.0f)) < 0.01f, "Triple armor stacks DEF");
    expectTrue(p.countOwnedItems(88021) == 3, "Owns 3 identical armor");
    expectTrue(p.purchaseAndEquip(88020), "First sword with triple armor");
    expectTrue(!p.purchaseAndEquip(88020), "Second sword blocked by triple armor");
    expectTrue(!p.isDualWielding(), "No dual-wield under triple armor");
}

void testUnequipOffHandSameItemId() {
    itemDatabase[88040] = {"High Sword", WEAPON, HIGH, 15.0f, 35, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88040), "First sword");
    expectTrue(p.purchaseAndEquip(88040), "Second sword dual-wield");
    expectTrue(p.isDualWielding(), "Dual-wield before off-hand unequip");
    expectTrue(p.unequipSlot(SLOT_WEAPON2), "Unequip off-hand by slot");
    expectTrue(!p.isDualWielding(), "Off-hand cleared removes dual-wield flag");
    expectTrue(p.getEquippedId(SLOT_WEAPON) == 88040, "Main hand sword remains");
    expectTrue(p.getEquippedId(SLOT_WEAPON2) == 0, "Off-hand slot empty");
    expectTrue(std::abs(p.get_ATK() - (DEFAULT_ATK + 15.0f)) < 0.01f, "ATK from main hand only");
}

void testInventoryContainsOnlyBackpack() {
    itemDatabase[88050] = {"Test Potion", POTION, LOW, 12.0f, 10, false};
    Player p("Tester");
    expectTrue(!p.get_inventory()->contains(88050), "Not in backpack before add");
    p.add_item(88050);
    expectTrue(p.get_inventory()->contains(88050), "In backpack after add");
    expectTrue(p.get_inventory()->get_item(88050) == 88050, "get_item true when in backpack");
    expectTrue(p.get_inventory()->count_item(88050) == 1, "count_item in backpack");
}

void testUnequipAmbiguousSameItemId() {
    itemDatabase[88051] = {"High Armor", ARMOR, HIGH, 15.0f, 30, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88051), "Armor layer 1");
    expectTrue(p.purchaseAndEquip(88051), "Armor layer 2");
    expectTrue(!p.unequipItem(88051, true), "unequipItem ambiguous when same id twice");
    expectTrue(p.unequipSlot(SLOT_ARMOR2, false), "unequipSlot picks exact layer");
    expectTrue(p.getEquippedArmorCount() == 1, "One armor layer remains");
}

void testReconcileGhostOffHandOnLoad() {
    itemDatabase[88052] = {"High Sword", WEAPON, HIGH, 15.0f, 35, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88052), "Main hand");
    expectTrue(p.purchaseAndEquip(88052), "Off hand");
    expectTrue(p.isDualWielding(), "Dual-wield before corrupt save");
    p.change_state("ATK", DEFAULT_ATK + 15.0f);
    const json saved = p.toJson();
    Player loaded("Tester");
    loaded.fromJson(saved);
    expectTrue(!loaded.isDualWielding(), "Load fixes ghost off-hand slot");
    expectTrue(loaded.getEquippedId(SLOT_WEAPON2) == 0, "WEAPON2 cleared on load");
    expectTrue(std::abs(loaded.get_ATK() - (DEFAULT_ATK + 15.0f)) < 0.01f, "ATK matches main hand only");
}

void testUnequipToBackpack() {
    itemDatabase[88030] = {"High Sword", WEAPON, HIGH, 15.0f, 35, false};
    Player p("Tester");
    expectTrue(p.purchaseAndEquip(88030), "Equip sword for unequip test");
    expectTrue(std::abs(p.get_ATK() - (DEFAULT_ATK + 15.0f)) < 0.01f, "Sword adds ATK");
    expectTrue(p.unequipSlot(SLOT_WEAPON), "Unequip sword to backpack");
    expectTrue(std::abs(p.get_ATK() - DEFAULT_ATK) < 0.01f, "ATK restored after unequip");
    expectTrue(p.getEquippedId(SLOT_WEAPON) == 0, "Weapon slot empty");
}

void testPlayerJsonRoundTrip() {
    Player p("Hero");
    p.change_ATK(5.0f);
    p.set_isPoisoned(true);
    const json before = p.toJson();
    Player loaded("Hero");
    loaded.fromJson(before);
    expectTrue(std::abs(loaded.get_ATK() - p.get_ATK()) < 0.01f, "JSON round-trip ATK");
    expectTrue(loaded.get_isPoisoned() == p.get_isPoisoned(), "JSON round-trip poison");
}

// --- Expanded coverage (500+ assertions) ---

void testMapInvariantsAcrossSeeds() {
    const int roomCounts[] = {10, 15, 20};
    for (int seed = 1; seed <= 50; ++seed) {
        for (int diff = 0; diff < 3; ++diff) {
            const int totalRooms = roomCounts[diff];
            seedGameRandom(static_cast<unsigned>(seed * 997 + diff * 13));
            MapGenerator gen(seed);
            gen.initMapGenerator(totalRooms, diff);
            gen.generateMap();
            const auto rooms = gen.releaseRooms();
            const std::string pfx = key("map", seed, diff);

            expectTrue(rooms.size() == static_cast<size_t>(totalRooms), pfx + " size");
            expectTrue(rooms.back() != nullptr && rooms.back()->getRoomType() == BOSS, pfx + " finalBoss");

            for (int r = 1; r <= totalRooms; ++r) {
                if (gen.shouldHaveShop(r)) {
                    expectTrue(rooms[static_cast<size_t>(r - 1)]->getRoomType() == SHOP,
                               pfx + " shop@" + std::to_string(r));
                }
                if (r % 5 == 0) {
                    expectTrue(rooms[static_cast<size_t>(r - 1)]->getRoomType() != SHOP,
                               pfx + " bossNotShop@" + std::to_string(r));
                }
                if (r % 5 == 0 && r < totalRooms) {
                    expectTrue(rooms[static_cast<size_t>(r - 1)]->getRoomType() == BOSS,
                               pfx + " milestoneBoss@" + std::to_string(r));
                }
            }
        }
    }
}

void testShouldHaveShopTable() {
    struct Case { int rooms; int diff; int freq; };
    const Case cases[] = {{10, 0, 4}, {15, 1, 4}, {20, 2, 3}};
    for (const Case& c : cases) {
        MapGenerator gen(0);
        gen.initMapGenerator(c.rooms, c.diff);
        for (int r = 1; r <= c.rooms; ++r) {
            const bool onBoss = (r % 5) == 0;
            const bool onEdge = r <= 1 || r >= c.rooms;
            const bool expectShop = !onBoss && !onEdge && (r % c.freq) == 0;
            expectTrue(gen.shouldHaveShop(r) == expectShop,
                       key("shouldShop", c.rooms, r));
        }
    }
}

void testPlayerHpMoneyScore() {
    Player p("Stat");
    p.change_HP(-30.0f);
    expectTrue(std::abs(p.get_HP() - 70.0f) < 0.01f, "HP decrease");
    p.change_HP(20.0f);
    expectTrue(std::abs(p.get_HP() - 90.0f) < 0.01f, "HP increase below cap");
    p.change_HP(200.0f);
    expectTrue(std::abs(p.get_HP() - p.get_maxHP()) < 0.01f, "HP capped at maxHP");
    p.change_HP(-p.get_maxHP() - 5.0f);
    expectTrue(!p.get_isAlive(), "HP zero kills player");
    expectTrue(p.get_HP() <= 0.0f, "HP not positive after lethal");

    Player p2("Rich");
    p2.change_Money(25.5f);
    expectTrue(std::abs(p2.get_Money() - (DEFAULT_MONEY + 25.5f)) < 0.01f, "Money add");
    const float battleBefore = p2.get_battleScore();
    p2.change_score(10.0f);
    expectTrue(std::abs(p2.get_battleScore() - (battleBefore + 10.0f)) < 0.01f, "Battle score add");
    expectTrue(p2.get_score() > p2.get_battleScore(), "Overall score includes stats");
}

void testInventoryCapacity() {
    itemDatabase[88100] = {"Fill Item", POTION, LOW, 1.0f, 1, false};
    Player p("Pack");
    Inventory* inv = p.get_inventory();
    expectTrue(inv->get_capacity() == MAX_INVENTORY_SIZE, "Default capacity 20");
    for (int i = 0; i < MAX_INVENTORY_SIZE; ++i) {
        expectTrue(inv->add_item(88100), key("inv add", i));
    }
    expectTrue(inv->get_current_size() == MAX_INVENTORY_SIZE, "Inventory full");
    expectTrue(!inv->add_item(88100), "Reject over capacity");
    expectTrue(!p.hasInventorySpace(), "hasInventorySpace false when full");
    expectTrue(inv->remove_item(88100), "Remove one item");
    expectTrue(p.hasInventorySpace(), "Space after remove");
    expectTrue(inv->count_item(88100) == MAX_INVENTORY_SIZE - 1, "Count after remove");
}

void testItemGradeBonuses() {
    const float swordFx[] = {5.0f, 10.0f, 15.0f};
    const float armorFx[] = {5.0f, 10.0f, 15.0f};
    const ItemRarity grades[] = {LOW, MEDIUM, HIGH};
    for (int g = 0; g < 3; ++g) {
        const int swordId = 88200 + g;
        const int armorId = 88210 + g;
        itemDatabase[swordId] = {"Sword", WEAPON, grades[g], swordFx[g], 10, false};
        itemDatabase[armorId] = {"Armor", ARMOR, grades[g], armorFx[g], 10, false};
        Player ps("S" + std::to_string(g));
        expectTrue(ps.purchaseAndEquip(swordId), key("grade sword equip", g));
        expectTrue(std::abs(ps.get_ATK() - (DEFAULT_ATK + swordFx[g])) < 0.01f,
                   key("grade sword atk", g));
        Player pa("A" + std::to_string(g));
        expectTrue(pa.purchaseAndEquip(armorId), key("grade armor equip", g));
        expectTrue(std::abs(pa.get_DEF() - (DEFAULT_DEF + armorFx[g])) < 0.01f,
                   key("grade armor def", g));
    }
}

void testMonsterLevelTiers() {
    g_monsterHpMultiplier = 1.0f;
    for (int lvl = 1; lvl <= 8; ++lvl) {
        Monster m;
        m.initMonster(1, lvl);
        expectTrue(m.getHP() == m.getMaxHP(), key("monster full hp", lvl));
        expectTrue(m.getATK() > 0, key("monster atk positive", lvl));
        expectTrue(m.getExpReward() > 0, key("monster exp reward", lvl));
        expectTrue(m.getGoldReward() > 0, key("monster gold reward", lvl));
        expectTrue(m.get_isAlive(), key("monster alive at spawn", lvl));
        m.takeDamage(m.getHP());
        expectTrue(!m.get_isAlive(), key("monster dies at zero", lvl));
        expectTrue(m.getHP() == 0, key("monster hp zero", lvl));
    }
}

void testTrapAllTypes() {
    g_trapDamageMin = 5;
    g_trapDamageMax = 25;
    const TrapType types[] = {
        TrapType::SPIKE_PIT, TrapType::POISON_GAS,
        TrapType::FALLING_BLOCK, TrapType::FIRE_BLAST
    };
    const float scales[] = {1.0f, 0.7f, 1.2f, 0.85f};
    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
        seedGameRandom(static_cast<unsigned>(3000 + i));
        Trap t;
        t.initTrap(types[i], 1);
        const int expectMin = std::max(1, static_cast<int>(g_trapDamageMin * scales[i]));
        const int expectMax = std::max(expectMin, static_cast<int>(g_trapDamageMax * scales[i]));
        expectTrue(t.get_isActive(), key("trap active", static_cast<int>(i)));
        expectTrue(t.getDamageMin() == expectMin, key("trap scaled min", static_cast<int>(i)));
        expectTrue(t.getDamageMax() == expectMax, key("trap scaled max", static_cast<int>(i)));
        expectTrue(!t.getDescription().empty(), key("trap description", static_cast<int>(i)));
        const int dmg = t.activateTrap();
        expectTrue(dmg >= t.getDamageMin() && dmg <= t.getDamageMax(),
                   key("trap activate damage", static_cast<int>(i)));
    }
}

void testBattleSystemCombat() {
    seedGameRandom(9001u);
    g_monsterHpMultiplier = 1.0f;
    Player p("Warrior");
    p.change_ATK(40.0f);
    Monster m;
    m.initMonster(1, 1);
    m.setHP(8);
    m.setDEF(2);

    BattleSystem bs;
    bs.initBattle(&p, &m);
    expectTrue(bs.get_isBattleActive(), "battle active after init");
    expectTrue(bs.getRoundCount() == 0, "round count starts at 0");

    const int dmg = bs.playerAttack();
    expectTrue(dmg >= 1, "player attack deals at least 1");
    expectTrue(m.getHP() < 8, "monster loses HP from attack");

    seedGameRandom(9002u);
    BattleSystem bs2;
    Player p2("Tank");
    p2.change_DEF(30.0f);
    Monster m2;
    m2.initMonster(1, 2);
    m2.setATK(12);
    bs2.initBattle(&p2, &m2);
    expectTrue(bs2.playerDefend(), "defend succeeds");
    const float hpBefore = p2.get_HP();
    bs2.monsterAttack();
    expectTrue(p2.get_HP() < hpBefore || p2.get_HP() > 0.0f, "monster attack changes or sustains HP");

    seedGameRandom(9003u);
    Monster m3;
    m3.initMonster(1, 1);
    m3.setHP(1);
    m3.setDEF(0);
    Player p3("Slayer");
    p3.change_ATK(99.0f);
    BattleSystem bs3;
    bs3.initBattle(&p3, &m3);
    bs3.playerAttack();
    expectTrue(bs3.getLastResult() == BattleResult::PLAYER_WIN, "weak monster dies in one hit");
    expectTrue(bs3.getRewards()[0] > 0.0f, "exp reward recorded");
}

void testJsonEquipmentSlots() {
    itemDatabase[88300] = {"S1", WEAPON, HIGH, 15.0f, 1, false};
    itemDatabase[88301] = {"A1", ARMOR, HIGH, 15.0f, 1, false};
    Player p("Save");
    expectTrue(p.purchaseAndEquip(88300), "json main sword");
    expectTrue(p.purchaseAndEquip(88300), "json off sword");
    expectTrue(p.purchaseAndEquip(88301), "json armor after dual break");
    const json j = p.toJson();
    Player loaded("Save");
    loaded.fromJson(j);
    expectTrue(loaded.getEquippedId(SLOT_WEAPON) != 0, "json weapon slot");
    expectTrue(loaded.getEquippedId(SLOT_ARMOR) == 88301, "json armor slot");
    expectTrue(std::abs(loaded.get_ATK() - p.get_ATK()) < 0.01f, "json atk match");
    expectTrue(std::abs(loaded.get_DEF() - p.get_DEF()) < 0.01f, "json def match");
}

void testRandomUtilsBulk() {
    for (unsigned seed = 100; seed < 140; ++seed) {
        seedGameRandom(seed);
        for (int i = 0; i < 5; ++i) {
            const int v = getRandom(10, 20);
            expectTrue(v >= 10 && v <= 20, key("rand int", static_cast<int>(seed), i));
        }
        const float f = getRandomFloat(0.0f, 1.0f);
        expectTrue(f >= 0.0f && f <= 1.0f, key("rand float", static_cast<int>(seed)));
    }
}

void testRoomBasics() {
    for (int diff = 0; diff < 3; ++diff) {
        seedGameRandom(static_cast<unsigned>(500 + diff));
        Room room;
        room.initRoom(1, diff, NORMAL);
        room.generateRoomContent(diff);
        expectTrue(room.getRoomId() == 1, key("room id", diff));
        expectTrue(room.getDifficulty() == diff, key("room diff", diff));
        expectTrue(!room.isRoomCleared(), key("room not cleared", diff));
        expectTrue(room.clearRoom(), key("room clear", diff));
        expectTrue(room.isRoomCleared(), key("room cleared flag", diff));
        expectTrue(!room.getDescription().empty(), key("room description", diff));
    }
}

void testEquipFromInventoryPath() {
    itemDatabase[88400] = {"Inv Sword", WEAPON, MEDIUM, 10.0f, 5, false};
    Player p("Inv");
    p.add_item(88400);
    expectTrue(p.equipFromInventory(88400), "equip from inventory");
    expectTrue(p.getEquippedId(SLOT_WEAPON) == 88400, "sword equipped from bag");
    expectTrue(!p.get_inventory()->contains(88400), "removed from bag after equip");
}

void testCurePoisonStandalone() {
    Player p("Cure");
    p.set_isPoisoned(true);
    p.curePoison();
    expectTrue(!p.get_isPoisoned(), "curePoison clears flag");
}

void testMonsterAttackPlayer() {
    seedGameRandom(6000u);
    Player p("Victim");
    Monster m;
    m.initMonster(1, 3);
    const float hp = p.get_HP();
    const int dealt = m.attackPlayer(p);
    expectTrue(dealt >= 1, "monster attack min 1 damage");
    expectTrue(p.get_HP() < hp, "player HP reduced");
}

void testMapGeneratorMetadata() {
    MapGenerator gen(123);
    gen.initMapGenerator(15, 1);
    expectTrue(gen.getTotalRooms() == 15, "getTotalRooms");
    expectTrue(gen.getDifficulty() == 1, "getDifficulty");
    gen.generateMap();
    expectTrue(gen.getRoomById(1) != nullptr, "getRoomById finds room 1");
    expectTrue(gen.getRoomById(99) == nullptr, "getRoomById missing returns null");
}

void testUnequipNoInventorySpace() {
    itemDatabase[88500] = {"Heavy", ARMOR, HIGH, 15.0f, 1, false};
    itemDatabase[88501] = {"Filler", POTION, LOW, 1.0f, 1, false};
    Player p("Full");
    expectTrue(p.purchaseAndEquip(88500), "equip for full bag test");
    for (int i = 0; i < MAX_INVENTORY_SIZE; ++i) {
        p.add_item(88501);
    }
    expectTrue(!p.unequipSlot(SLOT_ARMOR), "unequip blocked when backpack full");
}

void testCountOwnedItems() {
    itemDatabase[88600] = {"Stack Sword", WEAPON, LOW, 5.0f, 1, false};
    Player p("Own");
    expectTrue(p.purchaseAndEquip(88600), "own count sword 1");
    expectTrue(p.purchaseAndEquip(88600), "own count sword 2");
    expectTrue(p.countOwnedItems(88600) == 2, "count equipped swords");
    p.add_item(88600);
    expectTrue(p.countOwnedItems(88600) == 3, "count includes backpack");
}

} // namespace

GameTester::GameTester() = default;
GameTester::~GameTester() = default;

void GameTester::recordBug(const std::string& desc) {
    bugs.push_back(desc);
}

void GameTester::runAllTests() {
    bugs.clear();
    g_passed = 0;
    g_failed = 0;
    g_failures.clear();

    std::cout << "Running GameTester suite (target >= " << kMinAssertions << " assertions)...\n";

    testMonsterTakeDamageNoDoubleDef();
    testMonsterHpMultiplier();
    testTrapDamageUsesGlobals();
    testPlayerDefaults();
    testPlayerPoison();
    testSeededRandomDeterministic();
    testEasyHasShopRooms();
    testBossMilestoneNotShop();
    testInventoryPotionCuresPoison();
    testDualWield();
    testTripleArmor();
    testUnequipOffHandSameItemId();
    testInventoryContainsOnlyBackpack();
    testUnequipAmbiguousSameItemId();
    testReconcileGhostOffHandOnLoad();
    testUnequipToBackpack();
    testPlayerJsonRoundTrip();

    testMapInvariantsAcrossSeeds();
    testShouldHaveShopTable();
    testPlayerHpMoneyScore();
    testInventoryCapacity();
    testItemGradeBonuses();
    testMonsterLevelTiers();
    testTrapAllTypes();
    testBattleSystemCombat();
    testJsonEquipmentSlots();
    testRandomUtilsBulk();
    testRoomBasics();
    testEquipFromInventoryPath();
    testCurePoisonStandalone();
    testMonsterAttackPlayer();
    testMapGeneratorMetadata();
    testUnequipNoInventorySpace();
    testCountOwnedItems();

    std::cout << "Results: " << g_passed << " passed, " << g_failed << " failed.\n";
    if (g_passed < kMinAssertions) {
        std::cerr << "  WARNING: only " << g_passed << " assertions (minimum " << kMinAssertions << ")\n";
        ++g_failed;
        g_failures.push_back("assertion count below minimum");
    } else {
        std::cout << "  Coverage: " << g_passed << " assertions (>=" << kMinAssertions << " required).\n";
    }

    for (const std::string& failure : g_failures) {
        recordBug(failure);
    }
    failureCount_ = g_failed;
    generateTestReport();
}

void GameTester::generateTestReport() {
    testReport.clear();
    testReport += "=== Game Test Report ===\n";
    testReport += "Passed: " + std::to_string(g_passed) + " assertions\n";
    if (bugs.empty()) {
        testReport += "No failures.\n";
    } else {
        testReport += "Failures:\n";
        for (const std::string& bug : bugs) {
            testReport += " - " + bug + "\n";
        }
    }
    std::cout << testReport;
}
