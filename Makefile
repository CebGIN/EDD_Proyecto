# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Isrc -Isrc/external

# Project structure
SRC_DIR = src
BIN_DIR = bin
TARGET = $(BIN_DIR)/mafia_system

# Source files
SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/csvReader/csvReader.cpp \
          $(SRC_DIR)/mafiaFamily/mafiaTree.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
	@echo "Build successful! Executable is at $(TARGET)"

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Cleaned up."

# Run the project
run: all
	./$(TARGET)

.PHONY: all clean run
