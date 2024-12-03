CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

SRC_DIR = src
OBJ_DIR = obj
TEST_DIR = tests
TARGET = test.exe

SRC_FILES = $(SRC_DIR)/OrderBook.cpp $(SRC_DIR)/FixedWindowRateLimiter.cpp $(SRC_DIR)/TokenBucketRateLimiter.cpp $(SRC_DIR)/RiskControl.cpp $(SRC_DIR)/SelfCrossChecker.cpp
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
TEST_SRC = $(TEST_DIR)/test_RiskControl.cpp
TEST_OBJ = $(OBJ_DIR)/test_RiskControl.o

all: $(TARGET)

$(TARGET): $(OBJ_FILES) $(TEST_OBJ) | $(OBJ_DIR)
	$(CXX) $(OBJ_FILES) $(TEST_OBJ) -o $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/test_RiskControl.o: $(TEST_SRC) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

.PHONY: all clean
