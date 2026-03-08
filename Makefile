# Speed-Flip Cross-Platform Makefile (macOS + Windows MSYS2)

CC = gcc
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/game.c \
          $(SRC_DIR)/board.c \
          $(SRC_DIR)/logic.c \
          $(SRC_DIR)/io.c \
          $(SRC_DIR)/util.c \
          $(SRC_DIR)/render.c \
          $(SRC_DIR)/audio.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

EXECUTABLE = $(BUILD_DIR)/speed-flip

UNAME_S := $(shell uname -s)

.PHONY: all clean run

# ==========================
# macOS (Apple Silicon + Intel)
# ==========================
ifeq ($(UNAME_S),Darwin)

CFLAGS = -Wall -O2 -Isrc \
	-I/usr/local/include/SDL2

LDFLAGS = -L/usr/local/lib \
	-lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

EXECUTABLE = $(BUILD_DIR)/speed-flip

endif

# ==========================
# Windows (MSYS2 UCRT64)
# ==========================
ifeq ($(OS),Windows_NT)

CFLAGS = -Wall -O2 -Isrc -IC:/msys64/ucrt64/include/SDL2

LDFLAGS = -LC:/msys64/ucrt64/lib \
          -lmingw32 -lSDL2main -lSDL2 \
          -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

EXECUTABLE = $(BUILD_DIR)/speed-flip.exe

endif

# ==========================
# BUILD RULES
# ==========================

all: $(BUILD_DIR) $(EXECUTABLE)
	cp -r assets $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

# 빌드 + 실행
run: all
	./$(EXECUTABLE)
