#include "game.h"
#include "utils.h"

int main() {
    ensureDataDirectory();
    Game game;
    game.showMainMenu();
    return 0;
}
