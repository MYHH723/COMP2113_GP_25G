#ifndef GAMETESTER_H
#define GAMETESTER_H

#include <string>
#include <vector>

// GameTester class - Provides game testing functionality and test report generation
class GameTester {
private:
    // List to store all detected bug descriptions
    std::vector<std::string> bugs;

    // Final generated test report content
    std::string testReport;

    // Failed assertion count from the last runAllTests()
    int failureCount_ = 0;

public:
    // Default constructor
    GameTester();

    // Destructor
    ~GameTester();

    // Execute all available game test cases
    void runAllTests();

    // Add a new bug record to the bug list
    void recordBug(const std::string& desc);

    // Generate and output the complete test report
    void generateTestReport();

    // Number of failed assertions from the last runAllTests()
    int getFailureCount() const { return failureCount_; }
};

#endif // GAMETESTER_H
