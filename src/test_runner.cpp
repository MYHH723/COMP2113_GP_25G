#include "gametester.h"

int main() {
    GameTester tester;
    tester.runAllTests();
    return tester.getFailureCount() > 0 ? 1 : 0;
}
