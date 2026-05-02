// savegame.h
#ifndef SAVEGAME_H
#define SAVEGAME_H

// Include JSON library for game data serialization and deserialization
#include "third_party/json/single_include/nlohmann/json.hpp"

// Alias for JSON library to simplify code usage
using json = nlohmann::json;

// Global functions removed - save/load logic is now implemented as member functions of Game class

#endif
