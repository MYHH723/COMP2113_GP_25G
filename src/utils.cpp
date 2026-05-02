/**
 * @file utils.cpp
 * @brief Implementation of utility functions.
 * @author Member 1
 */

#include "utils.h"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <limits>
#include <sstream>
#include <iomanip>

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
    // If the buffer is empty, ignore(max,'\n') would block and consume the first Enter,
    // then get() would require a second Enter. Only discard a pending line when data is already buffered.
    std::streambuf* sb = std::cin.rdbuf();
    if (sb) {
        std::streamsize ready = sb->in_avail();
        if (ready > 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    std::cin.get();
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