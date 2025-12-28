# Makefile for libretro frontend using raylib

# Compiler
CC = clang

# Directories
RAYLIB_DIR = ../raylib/src
SRC_DIR = .
OBJ_DIR = obj

# Source files
SOURCES = main.c libretro_frontend.c
OBJECTS = $(SOURCES:%.c=$(OBJ_DIR)/%.o)

# Libraries
RAYLIB_LIB = $(RAYLIB_DIR)/libraylib_osx.a
LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreAudio -framework AudioToolbox -ldl

# Compiler flags
CFLAGS = -std=c99 -Wall -Wextra -O2
INCLUDES = -I$(RAYLIB_DIR) -I$(SRC_DIR)

# Output
TARGET = libretro_raylib

# Default target
all: $(TARGET)

# Create object directory
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) $(RAYLIB_LIB)
	$(CC) $(OBJECTS) $(RAYLIB_LIB) $(LIBS) -o $(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Phony targets
.PHONY: all clean install

