/**
 * @file utils.cpp
 * @brief Implementation of utility functions.
 * @author Member 1
 */

#include "utils.h"
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <limits>
#include <sstream>
#include <iomanip>
#include <string>
#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#include <conio.h>
#else
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

#ifdef _WIN32
void waitEnterSilently() {
    std::cin.clear();
    std::string line;
    if (std::getline(std::cin, line)) {
        return;
    }
    std::cin.clear();
    for (;;) {
        const int c = _getch();
        if (c == '\r' || c == '\n') break;
    }
}
#else
struct TermiosGuard {
    termios old{};
    bool ok = false;

    TermiosGuard() {
        if (!isatty(STDIN_FILENO)) return;
        if (tcgetattr(STDIN_FILENO, &old) != 0) return;
        termios t = old;
        // Raw-ish byte reads: avoids std::getline + terminal echo quirks on macOS/Linux terminals.
        t.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));
        t.c_cc[VMIN] = 1;
        t.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &t) != 0) return;
        ok = true;
    }

    ~TermiosGuard() {
        if (ok) tcsetattr(STDIN_FILENO, TCSANOW, &old);
    }

    TermiosGuard(const TermiosGuard&) = delete;
    TermiosGuard& operator=(const TermiosGuard&) = delete;
};

void waitEnterSilently() {
    if (!isatty(STDIN_FILENO)) {
        std::cin.get();
        return;
    }
    std::cout.flush();
    TermiosGuard guard;
    if (!guard.ok) {
        std::cin.get();
        return;
    }
    unsigned char ch = 0;
    while (true) {
        const ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n <= 0) break;
        if (ch == '\n' || ch == '\r') break;
    }
    std::cin.clear();
}
#endif

} // namespace

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}

void waitForEnter() {
    std::cout << "\nPress Enter to continue...\n" << std::flush;
    std::cin.clear();
    discardRestOfLineIfBuffered();
    waitEnterSilently();
}

void discardRestOfLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void discardRestOfLineIfBuffered() {
    std::cin.clear();
    std::streambuf* sb = std::cin.rdbuf();
    if (sb) {
        std::streamsize ready = sb->in_avail();
        if (ready > 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

bool promptYesNo(const char* prompt) {
    std::cout << prompt << std::flush;
    discardRestOfLineIfBuffered();
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    if (line.empty()) return false;
    const char c = line[0];
    return c == 'y' || c == 'Y' || c == '1';
}

std::string formatFixed2(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

int getRandom(int min, int max) {
    if (max <= min) return min;
    return min + std::rand() % (max - min + 1);
}

float getRandomFloat(float min, float max) {
    if (max <= min) return min;
    constexpr int kScale = 10000;
    const int roll = getRandom(0, kScale);
    return min + (max - min) * static_cast<float>(roll) / static_cast<float>(kScale);
}

bool getRandomChance(int percent) {
    if (percent <= 0) return false;
    if (percent >= 100) return true;
    return getRandom(1, 100) <= percent;
}

void seedGameRandom(unsigned int seed) {
    std::srand(seed);
}

bool ensureDataDirectory() {
#ifdef _WIN32
    return _mkdir("data") == 0 || errno == EEXIST;
#else
    return mkdir("data", 0755) == 0 || errno == EEXIST;
#endif
}

std::string getSaveFilePath() {
    return "data/save.json";
}

void printWithDelay(const std::string& text, int ms) {
    for (char c : text) {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    std::cout << std::endl;
}

void printMonsterEncounterArt(const std::string& monsterName) {
    if (monsterName == "Skeleton Warrior") {
        // Pixel skull (13~14 cols) matching reference: #/B = dark, W/. = light (printed as space).
        static const char* const kSkull[] = {
            "       #########        ", // ....#####....  (4+5+4)
            "     ##         ##      ", // ...#WWWWW#...
            "    #             #     ", // ..#WWWWWWW#..
            "  ##   ###   ###   ##   ", // .##WWWWWWW##.
            " ##  ####    ####    ## ", // .#WBBWBBW#. centered to 13 cols
            "   ##      #      ##    ",
            "     #           #      ", // .#WWWWWWWWW#.
            "     #    # #    #      ", // ..#WWWBWWW#..
            "     #    # #    #      ", // ...#WWWWW#...,  // ...#W#W#W#...
            "     #    # #    #      ",
            "      ##########        ",
        };
        for (const char* line : kSkull) {
            std::cout << "              " << line << "\n";
        }
        std::cout << std::flush;
        return;
    }
    if (monsterName == "Goblin") {
        std::cout << R"(
              ,-----.
             /  o o  \
            |    ^    |
             \  ---  /
           ___|     |___
          /    \   /    \
         |      '-'      |
)" << std::flush;
    }
}

void printVictoryArt(int difficulty) {
    const int di = std::max(0, std::min(2, difficulty));
    if (di == 0) {
        std::cout << R"(
################################################################################
################################################################################
##                                                                            ##
##    ********************************************************************    ##
##    *                                                                  *    ##
##    *     C  O  N  G  R  A  T  U  L  A  T  I  O  N  S  !               *    ##
##    *                                                                  *    ##
##    *              >>>  E A S Y   M O D E  <<<                         *    ##
##    *                                                                  *    ##
##    ********************************************************************    ##
##                                                                            ##
################################################################################
################################################################################
)" << std::endl;
        return;
    }
    if (di == 1) {
        std::cout << R"(
                        *
                       ~ ~ ~
                     .-------.
                    /  o   o  \          NORMAL MODE
                   |  ~~~~~~~  |        ~~~~~~~~~~~~
                    \  POP!   /         You did it! Celebrate!
                 ____'-------'____
                |  ~~~~~~~~~~~~~   |
                |___  champagne ____|
                    \___________/
                   ~~~ fizz  fizz ~~~
)" << std::endl;
        return;
    }
    std::cout << R"(
 __  __   _   _   _____     _      _  __ _____ _   __ __
 \ \/ /  | | | | / ____|   | |    | |/ /_   _| \ | |  _ \
  >  <   | |_| || (___    | |    | ' /  | | |  \| | | | |
 /_/\_\   \___/  \_____|   |____| |_|\_\ |_| |_| \_|____/

              Y O U   A R E   T H E   K I N G   !

                         .
                        / \
                       / | \
                      /  |  \
                     |   |   |
                     |  ===  |
                     |   |   |
                    /|   |   |\
                   / |   |   | \
                  /  |   |   |  \
                     |   |   |
                    /=========\
                   /___________\
                  [====HARD====]
)" << std::endl;
}
