# --- makefile for PipeWire Project ---

# Source files
SRCS := $(wildcard *.c)
# You can add more source files like: SRCS := main.c utils.c audio_processing.c

# Object files (compiled .o files from source files)
OBJS := $(patsubst %.c,bin/%.o,$(SRCS))

# Executable name
TARGET := sound_board.out

# --- Compiler and Flags ---
CC := clang
CFLAGS := -Wall -g2 -O0 -std=c23# -Wall enables all warnings, -g enables debugging info

# Use pkg-config to get PipeWire's CFLAGS and LIBS
PKG_CONFIG_CFLAGS := $(shell pkg-config --cflags libpipewire-0.3)
PKG_CONFIG_LIBS := $(shell pkg-config --libs libpipewire-0.3)

# Use pkg-config to get ffmpeg CFLAGS and LIBS
PKG_CONFIG_FFMPEG_CFLAGS := $(shell pkg-config --cflags libavformat libavcodec libavutil libswresample)
PKG_CONFIG_FFMPEG_LIBS := $(shell pkg-config --libs libavformat libavcodec libavutil libswresample)

# Combine all CFLAGS
ALL_CFLAGS := $(CFLAGS) $(PKG_CONFIG_CFLAGS) $(PKG_CONFIG_FFMPEG_CFLAGS)
ALL_LIBS := $(PKG_CONFIG_FFMPEG_LIBS) $(PKG_CONFIG_LIBS)
# --- Rules ---

.PHONY: all clean bear

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o bin/$(TARGET) $(ALL_LIBS)

bin/%.o: %.c
	$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) compile_commands.json

# Bear command to generate a compile_command.json file for the project
bear: 
	bear -- make all


