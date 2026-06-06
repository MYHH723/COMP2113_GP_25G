# Makefile for COMP2113_GP Roguelike Game

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -Iinclude -I.
DEBUGFLAGS = -std=c++11 -Wall -Wextra -g
LDFLAGS = 

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
DATA_DIR = data

# Platform-specific settings (Linux/macOS vs Windows MinGW/MSYS2)
ifeq ($(OS),Windows_NT)
    EXE_EXT = .exe
    RM_RF = if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR) & if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
    RUN_CMD = $(BIN_DIR)/game$(EXE_EXT)
    MKDIR = if not exist
else
    EXE_EXT =
    RM_RF = rm -rf $(BUILD_DIR) $(BIN_DIR)
    RUN_CMD = ./$(BIN_DIR)/game
    MKDIR = mkdir -p
endif

# Game sources — canonical list in build/game_sources.txt (keep in sync with build.bat)
GAME_MODULE_NAMES := $(strip $(shell cat build/game_sources.txt))
GAME_SOURCES := $(addprefix $(SRC_DIR)/,$(addsuffix .cpp,$(GAME_MODULE_NAMES)))
OBJECTS := $(GAME_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJECTS = $(BUILD_DIR)/gametester.o $(BUILD_DIR)/test_runner.o $(BUILD_DIR)/monster.o \
               $(BUILD_DIR)/player.o $(BUILD_DIR)/trap.o $(BUILD_DIR)/item.o \
               $(BUILD_DIR)/types.o $(BUILD_DIR)/utils.o $(BUILD_DIR)/balance.o \
               $(BUILD_DIR)/mapgenerator.o $(BUILD_DIR)/room.o $(BUILD_DIR)/battlesystem.o
EXECUTABLE = $(BIN_DIR)/game$(EXE_EXT)
TEST_EXECUTABLE = $(BIN_DIR)/gametest$(EXE_EXT)

# Targets
.PHONY: all build clean run debug help test

# Default target
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build successful! Executable: $@"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Create directories if they don't exist
$(BUILD_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
	@mkdir -p $(BUILD_DIR)
endif

$(BIN_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
else
	@mkdir -p $(BIN_DIR)
endif

$(DATA_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist $(DATA_DIR) mkdir $(DATA_DIR)
else
	@mkdir -p $(DATA_DIR)
endif

# Debug build
debug: CXXFLAGS = $(DEBUGFLAGS)
debug: clean $(EXECUTABLE)
	@echo "Debug build complete!"

# Run the game
run: $(EXECUTABLE) | $(DATA_DIR)
	$(RUN_CMD)

# Clean build artifacts
clean:
ifeq ($(OS),Windows_NT)
	@$(RM_RF)
else
	@$(RM_RF)
endif
	@echo "Clean complete!"

# Clean all including data
distclean: clean
	@rm -rf $(DATA_DIR)/*.json $(DATA_DIR)/*.log
	@echo "Distclean complete!"

# Display help
help:
	@echo "COMP2113_GP Makefile - Available targets:"
	@echo "  all        - Build the game (default)"
	@echo "  build      - Same as 'all'"
	@echo "  debug      - Build with debug symbols"
	@echo "  run        - Run the compiled game"
	@echo "  test       - Build and run unit tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  distclean  - Remove build artifacts and game data"
	@echo "  help       - Display this help message"

# Build alias
build: $(EXECUTABLE)

# Unit tests (GameTester)
$(TEST_EXECUTABLE): $(TEST_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o $@ $(LDFLAGS)
	@echo "Test build successful! Executable: $@"

test: $(TEST_EXECUTABLE)
	$(TEST_EXECUTABLE)
