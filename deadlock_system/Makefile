CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11
LIBS = -lncurses -lm

TARGET = deadlock_system
SRC_DIR = src
OBJ_DIR = obj
DATA_DIR = data
LOGS_DIR = logs

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/ui.c $(SRC_DIR)/logger.c
OBJECTS = $(OBJ_DIR)/main.o $(OBJ_DIR)/ui.o $(OBJ_DIR)/logger.o

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR) $(DATA_DIR) $(LOGS_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $@
	@echo "✓ Build successful: ./$(TARGET)"

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/ui.o: $(SRC_DIR)/ui.c $(SRC_DIR)/common.h $(SRC_DIR)/logger.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/logger.o: $(SRC_DIR)/logger.c $(SRC_DIR)/logger.h $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
	@echo "✓ Cleaned build files"

clean-all: clean
	rm -rf $(DATA_DIR)/* $(LOGS_DIR)/*
	@echo "✓ Cleaned all data and logs"

run: $(TARGET)
	@clear
	@./$(TARGET)

install:
	@echo "Installing ncurses development libraries..."
	@sudo apt-get update && sudo apt-get install -y libncurses5-dev libncursesw5-dev

help:
	@echo "Deadlock Detection System - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the project"
	@echo "  make run      - Build and run the system"
	@echo "  make clean    - Remove build files"
	@echo "  make clean-all- Remove build files, data, and logs"
	@echo "  make install  - Install required dependencies (Ubuntu/Debian)"
	@echo "  make help     - Show this help message"

.PHONY: all clean clean-all run install help directories