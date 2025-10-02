# --- makefile for PipeWire Project ---

# Source files
SRCS := $(wildcard *.cpp)
# You can add more source files like: SRCS := main.c utils.c audio_processing.c

# Object files (compiled .o files from source files)
OBJS := $(patsubst %.cpp,bin/%.o,$(SRCS))

# Executable name
TARGET := sound_board.out

# --- Compiler and Flags ---
CXX := clang++
CXXFLAGS := -Wall -Wextra -g2 -std=c++26 # -Wall enables all warnings, -g enables debugging info

# Use pkg-config to get PipeWire's CXXFLAGS and LIBS
PKG_CONFIG_CXXFLAGS := $(shell pkg-config --cflags libpipewire-0.3)
PKG_CONFIG_LIBS := $(shell pkg-config --libs libpipewire-0.3)

# Use pkg-config to get ffmpeg CXXFLAGS and LIBS
PKG_CONFIG_FFMPEG_CXXFLAGS := $(shell pkg-config --cflags libavformat libavcodec libavutil)
PKG_CONFIG_FFMPEG_LIBS := $(shell pkg-config --libs libavformat libavcodec libavutil)

# Combine all CXXFLAGS
ALL_CXXFLAGS := $(CXXFLAGS) $(PKG_CONFIG_CXXFLAGS) $(PKG_CONFIG_FFMPEG_CXXFLAGS)
ALL_LIBS := $(PKG_CONFIG_FFMPEG_LIBS) $(PKG_CONFIG_LIBS)
# --- Rules ---

.PHONY: all clean bear

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o bin/$(TARGET) $(ALL_LIBS)

bin/%.o: %.cpp
	$(CXX) $(ALL_CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) compile_commands.json

# Bear command to generate a compile_command.json file for the project
bear: 
	bear -- make all


