BLD_DIR = bld
TARGET = clox
CC = gcc
CFLAGS = -std=c11 -Wall -pedantic -Wextra -Wno-unused-parameter
CFLAGS = -O0 -g
SRC_DIR = src

HEADERS = $(wildcard $(SRC_DIR)/*.h)
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BLD_DIR)/%.o)

# Rule to make the target
$(TARGET): $(BLD_DIR)/$(TARGET)

# Rule to link the final executable
$(BLD_DIR)/$(TARGET): $(OBJECTS)
	mkdir -p $(BLD_DIR)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

# Rule to compile each source file
$(BLD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	mkdir -p $(BLD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile for debugging
debug: clean $(OBJECTS)
	mkdir -p $(BLD_DIR)
	$(CC) $(OBJECTS) -o $(BLD_DIR)/$(TARGET) $(CFLAGS) $(CFLAGS_DEBUG)

# Phony target for clean
.PHONY: clean
clean:
	rm -rf $(BLD_DIR)
