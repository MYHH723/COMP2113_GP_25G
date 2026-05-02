/**
 * @file utils.h
 * @brief Utility functions for UI and randomness.
 * @author Member 1
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>

void clearScreen();                       // clear terminal (Linux)
/** Wait for Enter (not named `pause` — conflicts with POSIX pause() in unistd.h on macOS). */
void waitForEnter();
/** Discard the rest of the current input line after a formatted `cin >>` (avoids junk like `1abc`). */
void discardRestOfLine();
/** If stdin already has buffered characters, discard until newline (safe before `getline`). */
void discardRestOfLineIfBuffered();
/** Format a float for display with exactly 2 digits after the decimal point. */
std::string formatFixed2(float value);
int getRandom(int min, int max);          // inclusive random int
void printWithDelay(const std::string& text, int ms); // slow print

/** ASCII art when entering battle vs Goblin / Skeleton Warrior (no-op for other names). */
void printMonsterEncounterArt(const std::string& monsterName);
/** Victory banner by difficulty: 0 Easy, 1 Normal, 2 Hard. */
void printVictoryArt(int difficulty);

#endif