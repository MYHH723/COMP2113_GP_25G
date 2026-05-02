#include "battlesystem.h"
#include "player.h"
#include "monster.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>

static std::mt19937 gen(std::random_device{}());

namespace {

const char* const kLinesAttackAfter[] = {
    "Your steel finds flesh; the beast reels from the blow.",
    "A true strike - honour guides the edge this day.",
    "The blade sings; crimson pays the toll of battle.",
    "You drive forward; iron answers your command.",
    "Through grit and dust, your weapon bites deep.",
    "The foe buckles - another verse in the song of war.",
    "Steel meets hide; the dungeon echoes your resolve.",
    "With measured wrath you carve your answer.",
};

const char* const kLinesDefendAfter[] = {
    "You brace behind shield and faith; the blow rings hollow.",
    "Stone-hearted guard - the enemy finds no easy quarry.",
    "Your stance holds like a castle wall in storm.",
    "Blow turned aside; prayer and plate share the burden.",
    "The strike expends itself upon your ready guard.",
    "You yield ground to none; the shield remembers every oath.",
    "A knight's patience outlasts the hammer of fate.",
    "Guard high, spirit higher - the tide breaks on you.",
};

const char* const kLinesCounterSuccess[] = {
    "Riposte! Fortune favours the bold.",
    "You turn death aside and answer with your own steel.",
    "The beast overreaches - you punish its arrogance.",
    "Quick as chapel bells at Matins, your counter lands.",
    "A duelist's breath - strike where they least expect.",
    "They lunge; you dance; steel finishes the verse.",
};

const char* const kLinesCounterFail[] = {
    "Your riposte falters; the opening costs dear.",
    "Too slow - the foe reads your intent and strikes true.",
    "The counter fails; courage alone cannot turn every blade.",
    "Balance lost - a costly lesson in the lists.",
    "Your timing slips; the dungeon claims its due.",
};

const char* const kLinesFleeSuccess[] = {
    "You slip into shadow - live to fight another dawn.",
    "Discretion, not cowardice - the wise warrior withdraws.",
    "Boots on stone; behind you, only echoes and breath.",
    "The corridor swallows you; survival is its own victory.",
    "You quit the field with heart still beating - there will be other songs.",
};

const char* const kLinesFleeFail[] = {
    "Escape denied - the enemy closes like a gate of iron.",
    "Your heel catches; fate insists on one more exchange.",
    "The way is barred; steel demands another reckoning.",
    "No passage yet - the beast will not release you cheaply.",
};

void printRandomLine(const char* const* lines, size_t count) {
    if (count == 0) return;
    std::uniform_int_distribution<size_t> pick(0, count - 1);
    std::cout << "  * " << lines[pick(gen)] << "\n";
}

} // namespace

BattleSystem::BattleSystem() 
    : player(nullptr), currentMonster(nullptr), round_count(0), 
      isBattleActive(false), playerDefending(false),
      lastResult(BattleResult::ONGOING), reward{0.0f, 0.0f, 0.0f} {
    battleLog.clear();
    battleLog.push_back("Battle system initialized");
}

BattleSystem::~BattleSystem() {
    player = nullptr;
    currentMonster = nullptr;
}

void BattleSystem::initBattle(Player* p, Monster* m) {
    player = p;
    currentMonster = m;
    round_count = 0;
    isBattleActive = true;
    playerDefending = false;
    lastResult = BattleResult::ONGOING;
    reward[0] = 0.0f;
    reward[1] = 0.0f;
    reward[2] = 0.0f;
    
    battleLog.clear();
    std::string log = "Battle started!";
    if (currentMonster) log += " Encounter: " + currentMonster->getName();
    battleLog.push_back(log);
}

void BattleSystem::startBattle() {
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing!");
        return;
    }
    
    isBattleActive = true;
    battleLog.push_back("=== Battle Start ===");
}

void BattleSystem::endBattle() {
    isBattleActive = false;
    
    std::string resultStr;
    switch(lastResult) {
        case BattleResult::PLAYER_WIN: resultStr = "Player Wins"; break;
        case BattleResult::PLAYER_LOSE: resultStr = "Player Loses"; break;
        case BattleResult::PLAYER_FLEE: resultStr = "Player Fled"; break;
        case BattleResult::ONGOING: resultStr = "Battle Interrupted"; break;
    }
    
    battleLog.push_back("Battle ended: " + resultStr);
    battleLog.push_back("=== Battle End ===");
}

BattleResult BattleSystem::executeBattleRound() {
    if (!isBattleActive) {
        battleLog.push_back("Error: No active battle");
        return lastResult;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        isBattleActive = false;
        return lastResult;
    }
    
    if (!currentMonster->get_isAlive()) {
        battleLog.push_back("Monster defeated! Player wins!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_WIN;
        return lastResult;
    }
    
    if (!player->get_isAlive()) {
        battleLog.push_back("Player defeated!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_LOSE;
        return lastResult;
    }
    
    round_count++;
    battleLog.push_back("--- Round " + std::to_string(round_count) + " ---");

    std::cout << "\n--- Your turn (Round " << round_count << ") ---\n";
    std::cout << "You: HP " << static_cast<int>(player->get_HP())
              << "  |  " << currentMonster->getName()
              << " HP: " << currentMonster->getHP() << "\n";
    std::cout << "1. Attack   2. Defend   3. Counter   4. Flee\n";
    std::cout << "Choice (1-4): " << std::flush;

    int choice = 0;
    while (true) {
        if (std::cin >> choice) {
            if (choice >= 1 && choice <= 4) break;
            std::cout << "Invalid. Enter 1-4: " << std::flush;
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Enter 1-4: " << std::flush;
        }
    }

    if (choice == 1) {
        playerAttack();
        if (currentMonster && currentMonster->get_isAlive() && isBattleActive) {
            monsterAttack();
        }
        printRandomLine(kLinesAttackAfter,
                        sizeof(kLinesAttackAfter) / sizeof(kLinesAttackAfter[0]));
    } else if (choice == 2) {
        playerDefend();
        monsterAttack();
        printRandomLine(kLinesDefendAfter,
                        sizeof(kLinesDefendAfter) / sizeof(kLinesDefendAfter[0]));
    } else if (choice == 3) {
        bool counterSuccess = playerCounter();
        if (!counterSuccess && currentMonster && currentMonster->get_isAlive() && isBattleActive) {
            monsterAttack(1.10f);
        }
        printRandomLine(counterSuccess ? kLinesCounterSuccess : kLinesCounterFail,
                        counterSuccess ? sizeof(kLinesCounterSuccess) / sizeof(kLinesCounterSuccess[0])
                                     : sizeof(kLinesCounterFail) / sizeof(kLinesCounterFail[0]));
    } else if (choice == 4) {
        if (playerFlee()) {
            printRandomLine(kLinesFleeSuccess,
                            sizeof(kLinesFleeSuccess) / sizeof(kLinesFleeSuccess[0]));
        } else {
            if (currentMonster && currentMonster->get_isAlive() && isBattleActive) {
                monsterAttack();
            }
            printRandomLine(kLinesFleeFail,
                            sizeof(kLinesFleeFail) / sizeof(kLinesFleeFail[0]));
        }
    }
    return lastResult;
}

int BattleSystem::playerAttack() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return 0;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        return 0;
    }
    
    float playerAtk = player->get_ATK();
    std::uniform_real_distribution<float> atkDist(0.8f, 1.2f);
    float damageMultiplier = atkDist(gen);
    int baseDamage = static_cast<int>(playerAtk * damageMultiplier);
    
    int monsterDef = currentMonster->getDEF();
    int actualDamage = baseDamage - monsterDef;
    if (actualDamage < 1) actualDamage = 1;
    
    battleLog.push_back("Player attacks " + currentMonster->getName() + 
                       ", dealing " + std::to_string(actualDamage) + " damage");
    
    currentMonster->takeDamage(actualDamage);
    
    if (!currentMonster->get_isAlive()) {
        battleLog.push_back("Defeated " + currentMonster->getName() + "!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_WIN;
        
        float reward_exp = static_cast<float>(currentMonster->getExpReward());
        float reward_gold = static_cast<float>(currentMonster->getGoldReward());
        float reward_score = currentMonster->getScoreReward();
        
        reward[0] += reward_exp;
        reward[1] += reward_gold;
        reward[2] += reward_score;
        
        battleLog.push_back("Gained " + std::to_string(reward_exp) + " EXP and " + 
                std::to_string(reward_gold) + " Gold");
    }
    battleLog.push_back("Monster remaining HP: " + std::to_string(currentMonster->getHP()));
    return actualDamage;
}

int BattleSystem::monsterAttack(float extraDamageMultiplier) {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return 0;
    }
    
    if (!player || !currentMonster) {
        battleLog.push_back("Error: Player or monster missing");
        return 0;
    }
    
    std::uniform_int_distribution<int> damageDist(
        static_cast<int>(currentMonster->getATK() * 0.8f),
        static_cast<int>(currentMonster->getATK() * 1.2f)
    );
    int baseDamage = damageDist(gen);

    float effectiveDef = player->get_DEF();
    if (playerDefending) {
        effectiveDef *= 1.8f;
    }

    int damage = baseDamage - static_cast<int>(effectiveDef);
    if (damage < 1) damage = 1;
    damage = static_cast<int>(damage * extraDamageMultiplier);
    if (damage < 1) damage = 1;
    player->change_HP(-static_cast<float>(damage));
    playerDefending = false;
    
    battleLog.push_back("Player remaining HP: " + std::to_string(static_cast<int>(player->get_HP())));
    
    if (damage > 0) {
        battleLog.push_back(currentMonster->getName() + " attacks player, dealing " + 
                           std::to_string(damage) + " damage");
        
        if (!player->get_isAlive()) {
            battleLog.push_back("Player defeated!");
            isBattleActive = false;
            lastResult = BattleResult::PLAYER_LOSE;
        }
    }
    
    return damage;
}

bool BattleSystem::playerFlee() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return false;
    }
    
    std::uniform_int_distribution<int> fleeChance(1, 100);
    int chance = fleeChance(gen);
    
    if (chance <= 70) {
        battleLog.push_back("Player successfully fled!");
        isBattleActive = false;
        lastResult = BattleResult::PLAYER_FLEE;
        return true;
    } else {
        battleLog.push_back("Failed to flee!");
        return false;
    }
}

bool BattleSystem::playerDefend() {
    if (!isBattleActive) {
        battleLog.push_back("Error: Battle not started");
        return false;
    }
    
    playerDefending = true;
    battleLog.push_back("Player chooses to defend (defense boosted this round)");
    return true;
}

bool BattleSystem::playerCounter() {
    if (!isBattleActive || !player || !currentMonster) return false;

    // Counter chance scales with ATK and enemy difficulty proxy.
    float atkFactor = player->get_ATK() / 250.0f;
    float difficultyPenalty = 0.10f;
    int enemyLevel = currentMonster->getLevel();
    if (enemyLevel >= 5) difficultyPenalty = 0.18f;
    else if (enemyLevel >= 3) difficultyPenalty = 0.14f;

    float successRate = 0.25f + atkFactor - difficultyPenalty;
    if (successRate < 0.08f) successRate = 0.08f;
    if (successRate > 0.65f) successRate = 0.65f;

    std::uniform_real_distribution<float> roll(0.0f, 1.0f);
    if (roll(gen) <= successRate) {
        battleLog.push_back("Counter success! No damage taken.");
        playerAttack();
        int bonusGold = std::max(1, static_cast<int>(currentMonster->getGoldReward() * 0.05f));
        reward[1] += static_cast<float>(bonusGold);
        battleLog.push_back("Counter bonus: +" + std::to_string(bonusGold) + " gold.");
        return true;
    }

    battleLog.push_back("Counter failed! Incoming damage increased by 10%.");
    return false;
}

BattleResult BattleSystem::getLastResult() const { return lastResult; }
int BattleSystem::getRoundCount() const { return round_count; }
bool BattleSystem::get_isBattleActive() const { return isBattleActive; }
std::vector<std::string> BattleSystem::getBattleLog() const { return battleLog; }

const float* BattleSystem::getRewards() const {
    return reward;
}

std::string BattleSystem::showBattleLog() {
    std::stringstream ss;
    ss << "=== Battle Log ===" << std::endl;
    
    if (battleLog.empty()) {
        ss << "(No battle log)" << std::endl;
    } else {
        for (const auto& log : battleLog) {
            ss << log << std::endl;
        }
    }
    
    return ss.str();
}

void BattleSystem::applyRewards() {
    if (player) {
        player->change_EXP(reward[0]);
        player->change_Money(reward[1]);
        player->change_score(reward[2]);
    }
}
