# --- makefile for PipeWire Project ---

# Source files
SRCS := $(wildcard src/*.c)

# Object files (compiled .o files from source files)
OBJS := $(patsubst src/%.c, bin/%.o,$(SRCS))

# Executable name
TARGET := sound_board.out

# --- Compiler and Flags ---
CC := gcc 
CFLAGS := -Wall -g2 -O0 -pthread -std=c23

PKG_CONFIG_PIPEWIRE_CFLAGS := $(shell pkg-config --cflags libpipewire-0.3)
PKG_CONFIG_PIPEWIRE_LIBS := $(shell pkg-config --libs libpipewire-0.3)

PKG_CONFIG_FFMPEG_CFLAGS := $(shell pkg-config --cflags libswresample libavformat libavcodec libavutil)
PKG_CONFIG_FFMPEG_LIBS := $(shell pkg-config --libs libswresample libavformat libavcodec libavutil)

# Combine all CFLAGS
ALL_CFLAGS := $(CFLAGS) $(PKG_CONFIG_PIPEWIRE_CFLAGS) $(PKG_CONFIG_FFMPEG_CFLAGS)
ALL_LIBS := $(PKG_CONFIG_FFMPEG_LIBS) $(PKG_CONFIG_PIPEWIRE_LIBS)
# --- Rules ---

.PHONY: all clean bear

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o bin/$(TARGET) $(ALL_LIBS)

bin/%.o: src/%.c
	$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) compile_commands.json

# Bear command to generate a compile_command.json file for the project which works with clangd lsp support in my nvim
bear: 
	bear -- make all

