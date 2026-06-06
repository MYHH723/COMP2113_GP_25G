A list of non-standard C/C++ libraries, if any, that are used in your work and integrated to your
code repo. Please also indicate what features in your game are supported by these libraries.

# COMP2113_GP - Roguelike Dungeon Game

A C++ based roguelike dungeon exploration game with difficulty scaling, combat system, merchant shop.

## Group Member

- Member1: Li Zhaotian 3033120221 
- Member2: Zou Jiachen 3036481016
- Member3: Lin Shuhong 3036666513 
- Member4: Wang Liheng 3036401523
- Member5: Jin Haoyue 3036529882

## Project Discription

This is a text-based roguelike game where players navigate through procedurally generated dungeon rooms, fight monsters, avoid traps, manage inventory, and purchase items from a merchant. The game features three difficulty levels with scaled monster/trap difficulty.

### Features

- **Difficulty System**: Easy, Normal, and Hard modes with scaled enemy stats and trap damage
- **Combat System**: Turn-based battle mechanics with attack, defend, and flee options
- **Room Generation**: Procedurally generated dungeons with shops, treasure rooms, and boss encounters
- **Player Progression**: Experience system, leveling, stat management, and save/load functionality
- **Shop System**: Buy/sell items with difficulty-based pricing and transaction logging

### Coding Requirements Matching

| Coding Element | How It Supports the Features |
|----------------|------------------------------|
| **Generation of random events** | `mapgenerator.cpp` randomly determines room types (shop / treasure / boss / empty). `battlesystem.cpp` uses random damage rolls for attacks. `room.cpp` randomly spawns traps or events. This powers the **Room Generation** and **Combat System** features. |
| **Data structures for storing data** | `player.h` defines `Player` and `Inventory` structs (using `std::vector` or arrays). `Item`, `Room`, and `Monster` data structures. 
`type` defines struct and enum to store data. These organize all game state data efficiently, supporting **Player Progression** and **Shop System**. |
| **Dynamic memory management** | Rooms, monsters, traps, player inventory, and map data use `std::unique_ptr` for ownership (`MapGenerator::releaseRooms()` hands rooms to `Game`). No manual `delete` in game flow. |
| **File input/output** | `game_save.cpp` uses the `nlohmann/json` library (from `third_party/json/`) to read/write `data/save.json`. Player health, level, inventory, and current room are serialized and loaded, directly supporting the **Player Progression** save/load functionality. |
| **Program codes in multiple files** | The project is split into modular `.cpp` and `.h` files: `game.cpp`, `player.cpp`, `battlesystem.cpp`, `room.cpp`, `mapgenerator.cpp`, `merchant.cpp`, `shop.cpp`, etc. This separation of concerns makes the code maintainable and supports **all five features**. |
| **Multiple Difficulty Levels** | Difficulty selected at startup in `main.cpp` or `game.cpp` affects monster stats (enemy scaling), trap damage, room ammount. Implemented in `battlesystem.cpp` (combat formulas), `trap.cpp` (damage calculation), `monster.cpp` (attribute multiplier), `game,cpp` (room ammount). This directly supports the **Difficulty System** feature. |

### Non-Standard Libraries Used

| Library | Version | Location in Repo | Integration Method | Features Supported |
|---------|---------|-----------------|-------------------|-------------------|
| **nlohmann/json** | Header-only (v3.11.2+) | `third_party/json/single_include/nlohmann/json.hpp` | Git submodule; included via `-I./third_party/json/single_include` in Makefile | **Player Progression**: JSON serialization/deserialization for save/load functionality. Player stats (HP, ATK, DEF, EXP, Money), inventory items, game state (difficulty, current room, win/lose status) are persisted to `data/save.json`. |

> **Note**: All other functionality uses only standard C++ libraries: `<iostream>`, `<fstream>`, `<string>`, `<vector>`, `<list>`, `<map>`, `<algorithm>`, `<random>`, `<ctime>`, `<chrono>`, `<cmath>`, `<limits>`, `<sstream>`, `<iomanip>`, `<cstdlib>`, and `<filesystem>` .

## Compilation & Execution Instruction

### Clone the Repository

```bash
# Clone with submodules (recommended - includes third_party/json)
git clone --recursive https://github.com/MYHH723/COMP2113_GP_25G
cd COMP2113_GP_25G

# Or if already cloned without --recursive
git submodule update --init --recursive

# Build and run
make          # Build optimized version
make run      # Build and run the game
```

### Windows

1. Install a C++ compiler (pick one):
   - [MSYS2](https://www.msys2.org/): open **MSYS2 MinGW 64-bit**, then `pacman -S mingw-w64-x86_64-gcc make`
   - [MinGW-w64](https://www.mingw-w64.org/) standalone and add `g++` to PATH
2. Initialize the JSON submodule (same as above).
3. Open **cmd** or **PowerShell** in the project folder and run:

```bat
build.bat
bin\game.exe
build.bat test
```

### Release build (distributable zip)

Static-linked executable for Windows (no MinGW DLLs required):

```bat
build-release.bat
package.bat
```

Output:

- `bin\RoguelikeGame.exe` — standalone game
- `release\RoguelikeGame.zip` — send this to others (includes `data\` folder for saves)

> Source file list for all Windows builds: `build/game_sources.txt` (shared with `Makefile`).

Or, from **MSYS2 MinGW 64-bit** terminal:

```bash
make
make run
```

> Prefer **cmd** or **Windows Terminal** for keyboard input. MSYS2/Mintty also works (Enter uses line-based input on Windows).

### Between rooms

- `[C]` — continue to the next room (empty Enter does **not** advance)
- `[I]` — view inventory
- `[U]` — use potion (heals + cures poison) or equip weapon/armor from backpack
- `[E]` — unequip worn gear to backpack (pick slot: main hand, off hand, armor layers)

Status panel shows `(dual-wield)` or `(triple armor)` when active, plus an **Equipped** slot list.

## Platform Support

- Linux Terminal (primary target)
- SSH Environment
- Mac (with proper terminal support)
- Windows (MinGW-w64 / MSYS2; see above)

## Game Flow

1. **Main Menu**: Player selects New Game, Load Game, or Exit
2. **Difficulty Selection**: Choose Easy/Normal/Hard
3. **Game Loop**:
   - Enter next room
   - Encounter monsters and/or traps
   - Battle or shop interactions
   - Room cleared → Progress to next room
4. **Game End**: Win (all rooms cleared) or Lose (player HP = 0)

## Project Structure

```
COMP2113_GP/
├── src/                      # Source code files
│   ├── consoleUI.cpp         # Console UI implementation
│   ├── game.cpp              # Game core logic
│   ├── main.cpp              # Program entry point 
│   ├── room.cpp              # Room management 
│   ├── battlesystem.cpp      # Combat mechanics 
│   ├── item.cpp              # Item definitions 
│   ├── gamelogger.cpp        # Transaction logging 
│   ├── trap.cpp              # Trap mechanics 
│   ├── types.cpp             # Type definitions 
│   ├── utils.cpp             # Utility functions 
│   ├── monster.cpp           # Monster behaviors 
│   ├── mapgenerator.cpp      # Dungeon generation 
│   ├── merchant.cpp          # Merchant logic 
│   ├── shop.cpp              # Shop transactions 
│   ├── savegame.cpp          # Progression saving 
│   └── player.cpp            # Player implementation 
│
├── include/                  # Header files
│   ├── game.h                # Game core control 
│   ├── player.h              # Player, Inventory, Panel classes 
│   ├── consoleUI.h           # Console UI header 
│   ├── types.h               # Type definitions 
│   ├── room.h                # Room management 
│   ├── mapgenerator.h        # Dungeon generation 
│   ├── monster.h             # Enemy monsters 
│   ├── trap.h                # Room traps 
│   ├── battlesystem.h        # Combat mechanics 
│   ├── item.h                # Item definitions 
│   ├── merchant.h            # Merchant/goods 
│   ├── shop.h                # Shop transactions 
│   ├── gamelogger.h          # Transaction logging 
│   └── gametester.h          # Testing framework 
│
├── third_party/              # External dependencies 
│   └── json/                 # nlohmann/json library 
│
├── data/                     # Game data storage 
│   └── save.json             # Player save file 
│
├── docs/                     # Documentation 
│
├── Makefile                  # Build configuration 
│
└── README.md                 # This file 
```

## Module Breakdown

### Member 1: Game Core (game.h, main.h)

- Main game loop and flow control
- Difficulty selection and initialization
- Menu system and game state management
- Utility functions (random, delays, screen clearing)
- Global difficulty constants and multipliers

### Member 2: Player System (player.h, savegame.h)

- **Player Class**: Manages HP, ATK, DEF, EXP, Money, status effects
- **Inventory Class**: Item storage with capacity limits (max 20 items)
- Save/load functionality to JSON

### Member 3: Combat System (monster.h, trap.h, battlesystem.h)

- **Monster Class**: Enemy entities with scalable difficulty stats
- **Trap Class**: Environmental hazards (Spike Pit, Poison Gas, Falling Block, Fire Blast)
- **BattleSystem Class**: Turn-based combat with attack/defend/flee mechanics
- Damage calculations with variance
- Battle logging and result tracking

### Member 4: Room & Map System (consoleUI.h, room.h, mapgenerator.h)

- **ConsoleUI Class**: UI demonstrating player's status and inventory level
- **Room Class**: Individual dungeon rooms with content and state tracking
- **MapGenerator Class**: Procedural dungeon generation
- Room types: Normal, Boss, Shop, Treasure
- Difficulty-based content scaling

### Member 5: Shop System (item.h, merchant.h, shop.h, gamelogger.h, gametester.h)

- **Item Class**: Three types (Potion, Sword, Armor) with three grades (Low/Mid/High)
- **Merchant Class**: Goods management with unlimited stock
- **Shop Class**: Buy/sell transactions with gold management
- **GameLogger Class**: Transaction logging
- **GameTester Class**: Testing and bug reporting

## Difficulty Scaling

| Difficulty | Total Rooms | Monster HP | Trap Damage | Shop Frequency  |
| ---------- | ----------- | ---------- | ----------- | --------------- |
| Easy       | 10          | 0.8x       | 5–12        | Every 4 rooms (e.g. room 4, 8; skips Boss rooms) |
| Normal     | 15          | 1.0x       | 8–18        | Every 4 rooms                 |
| Hard       | 20          | 1.3x       | 12–25       | Every 3 rooms                 |

**Room layout rules** (`mapgenerator.cpp`):

- Final room is always **Boss**
- Every 5th room (except the last) is **Boss**
- Shops appear when `roomNumber % shopFrequency == 0`, but never on Boss milestones (every 5th room)
- ~12% chance for **Treasure** among remaining rooms; otherwise **Normal**

Run unit tests: `make test` or `build.bat test` (2000+ assertions; map seeds, combat, equipment, save/load).

## Player Stats

Default values are defined in `include/player.h` (`DEFAULT_HP`, `DEFAULT_ATK`, etc.):

- **HP**: 100 (default), affected by damage/healing
- **ATK**: 10 (default), affects damage dealt
- **DEF**: 10 (default), reduces damage taken
- **EXP**: Experience points for leveling
- **Money**: 0 gold at new game start (earn from combat and treasure)
- **level**: Player's grade
- **score**: Summaries of battle and player attribute
- **Poison**: 30% chance on trap trigger; deals 3 HP per combat round until cured (saved in `save.json`)

## Item System

### Potions (Consumable)
- **Effect**: Heals by grade (Low +12, Medium +16, High +22 HP); cures poison
- **Usage**: Bought into backpack; shop asks **Use potion now? (y/n)**. Also usable via `[U]` between rooms
- **Price**: Base price × difficulty multiplier (Easy: 0.8x, Normal: 1.0x, Hard: 1.3x)

### Swords (Equipment)
- **Effect**: ATK bonus by grade (Low +5, Medium +10, High +15)
- **Dual-wield**: 2nd sword equips to off-hand (`WEAPON2`); **cannot wear armor** while dual-wielding
- **Usage**: Bought into backpack; shop asks **Equip now? (y/n)**. Equip via `[U]` or shop prompt
- **4th sword**: Replaces off-hand; previous off-hand returns to backpack

### Armor (Equipment)
- **Effect**: DEF bonus by grade (Low +5, Medium +10, High +15)
- **Layered armor**: Up to 3 layers (`ARMOR`, `ARMOR2`, `ARMOR3`); **cannot dual-wield** with 3 layers (single sword only)
- **Usage**: Bought into backpack; shop asks **Equip now? (y/n)**. Equipping armor stows off-hand sword to backpack
- **4th armor**: Replaces outermost layer (layer 3)

### Equipment rules (summary)
| Mode | Swords | Armor |
|------|--------|-------|
| Normal | 1 main hand | 0–3 layers |
| Dual-wield | Main + off hand | None |
| Triple armor | Main hand only | 3 layers |

Unequip: `[E]` between rooms, or shop menu **5. Unequip worn gear**. Always picks a **slot** (important when multiple items share the same merchant item ID).

### Shop Mechanics
- **Buying**: Pay gold → item goes to backpack → optional equip/use prompt; shows **You own Nx** count
- **Selling**: 50% of base price; grade selection; `0=Back` cancels
- **Menu**: Buy sword/potion/armor, sell, unequip, leave (`0`)
- **Merchant Dialogue**: Random medieval flavor text per action

### Difficulty Pricing
| Difficulty | Price Multiplier |
|------------|------------------|
| Easy (0)   | 0.8x             |
| Normal (1) | 1.0x             |
| Hard (2)   | 1.3x             |

## Trap Mechanics

- **When**: If a room rolled traps at generation, they trigger at the start of **Normal** / **Boss** / **Treasure** rooms, before combat (not every combat room has traps)
- **Spawn chance** (Normal rooms): Easy 35%, Normal 45%, Hard 55%; Boss rooms 50–70% by difficulty
- **Types**: Spike Pit, Poison Gas, Falling Block, Fire Blast (random per room)
- **Damage**: Scaled by difficulty via `g_trapDamageMin` / `g_trapDamageMax` in `game.cpp` (see table below), with per-type multipliers in `trap.cpp`
- **One-shot**: Each trap fires once per room entry, then becomes inactive
- **Poison**: 30% chance; 3 HP damage per combat round (see Player Stats)

| Difficulty | Trap Damage Range (base) |
|------------|--------------------------|
| Easy       | 5 – 12                   |
| Normal     | 8 – 18                   |
| Hard       | 12 – 25                  |

Monster HP uses `g_monsterHpMultiplier`: Easy **0.8×**, Normal **1.0×**, Hard **1.3×** (applied in `monster.cpp`).

## Combat Mechanics

- **Turn Order**: Player acts first → Monster responds (counter can interrupt)
- **Attack**: `Damage = max(1, Player_ATK × (0.8~1.2) - Monster_DEF)`
- **Defend**: 80% DEF boost for the round, then monster attacks
- **Counter**: 25-65% success chance (scales with ATK, penalized by enemy level). Success = deal attack + bonus gold; Fail = take 10% increased damage
- **Flee**: 70% success rate; on fail, monster attacks
- **Minimum Damage**: 1 (cannot be reduced below 1)
- **Rewards**: EXP, Gold, and Score from defeated monsters
- **Battle End**: Player Win / Player Lose / Flee Success
- **Random Elements**: Damage variance (±20%), counter success rate, flee outcome, battle flavor text



## Data Files

- **save.json**: Player save state (auto-generated in `data/` directory)
  - **saveVersion 3**: room layout always regenerated from `Seed` (current map rules); restores `cleared`, monster `hp`/`alive`, trap `active`/`triggerCount`, and player `itemDefs`
  - **Seed**: map is regenerated from `Seed` on load; room state is restored from the save payload
- **shop_transaction.log**: Transaction history (auto-generated)

## Future Enhancements

- Additional trap types and mechanics
- Boss-specific abilities
- Skill system for players
- Multiplayer/competitive modes
- Advanced loot/rare items
- UI improvements

## Team Members

- Member 1: Game Core & Flow
- Member 2: Player System & Inventory
- Member 3: Combat System
- Member 4: UI & Room & Map Generation
- Member 5: Shop & Item System

## Notes

- Game runs in terminal environment with text-based UI
- All game data is persisted to save.json
- Transaction logging helps with debugging shop interactions
- Difficulty affects both enemy stats and game duration
- No inventory limit for Potions (consumables)
- Equipment (Sword/Armor) stored in inventory with 20-item capacity
