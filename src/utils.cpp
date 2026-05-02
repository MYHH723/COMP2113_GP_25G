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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace {

#ifdef _WIN32
/** Prefer Console API so Mintty/Cursor terminals still suppress junk keys when attached to console. */
void waitEnterSilently() {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    if (hIn != INVALID_HANDLE_VALUE && hIn != nullptr && GetConsoleMode(hIn, &mode)) {
        const DWORD saved = mode;
        DWORD tweaked = mode & ~(DWORD)(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
        if (SetConsoleMode(hIn, tweaked)) {
            char ch = 0;
            DWORD nr = 0;
            while (ReadConsoleA(hIn, &ch, 1, &nr, nullptr) && nr == 1) {
                if (ch == '\r' || ch == '\n') break;
            }
            SetConsoleMode(hIn, saved);
            std::cin.clear();
            return;
        }
        SetConsoleMode(hIn, saved);
    }
    for (;;) {
        const int c = _getch();
        if (c == '\r' || c == '\n') break;
    }
    std::cin.clear();
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

void pause() {
    std::cout << "\nPress Enter to continue..." << std::flush;
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

std::string formatFixed2(float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

int getRandom(int min, int max) {
    return min + std::rand() % (max - min + 1);
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
            "    #####    ",   // ....#####....  (4+5+4)
            "   #     #   ",  // ...#WWWWW#...
            "  #       #  ",  // ..#WWWWWWW#..
            " ##       ## ",  // .##WWWWWWW##.
            "  # ## ## #  ",  // .#WBBWBBW#. centered to 13 cols
            "  # ## ## #  ",
            " #         # ",  // .#WWWWWWWWW#.
            "  #    #   #  ",  // ..#WWWBWWW#..
            "   #     #   ",  // ...#WWWWW#...
            "   # # # #   ",  // ...#W#W#W#...
            "   # # # #   ",
            "    #####    ",
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
##    *     C  O  N  G  R  A  T  U  L  A  T  I  O  N  S  !              *    ##
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