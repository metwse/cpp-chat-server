PROJECT_NAME = chatd

CC = gcc
CXX = g++
DEBUGGER = gdb
MEMLEAK_TESTER = valgrind

C_FLAGS=-O2 -Wall -Werror -D_GNU_SOURCE -std=gnu17 -I.
T_FLAGS=-O0 -g3 -Wall -std=gnu17 -D_DEBUG -D_GNU_SOURCE -I.

CXX_FLAGS=-O2 -Wall -Werror -std=c++17 -I.
TXX_FLAGS=-O0 -g3 -Wall -std=c++17 -D_DEBUG -I.

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_DIR = $(PROJECT_NAME)
BUILD_DIR = target
OBJ_DIR = $(BUILD_DIR)/obj

SRC_C = $(call rwildcard,$(SRC_DIR)/,*.c)
SRC_CXX = $(call rwildcard,$(SRC_DIR)/,*.cpp)


# Pattern rules for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/%.d
	$(CC) -c $(C_FLAGS) $< -o $@

$(OBJ_DIR)/%.oxx: $(SRC_DIR)/%.cpp | $(OBJ_DIR)/%.d
	$(CXX) -c $(CXX_FLAGS) $< -o $@

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/%.d
	$(CC) -c $(T_FLAGS) $< -o $@

$(OBJ_DIR)/%.debug.oxx: $(SRC_DIR)/%.cpp | $(OBJ_DIR)/%.d
	$(CXX) -c $(TXX_FLAGS) $< -o $@


# Source files that will included to final executable
OBJS_C = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_C))
DEBUG_OBJS_C = $(patsubst %.o,%.debug.o,$(OBJS_C))
OBJS_CXX = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.oxx,$(SRC_CXX))
DEBUG_OBJS_CXX = $(patsubst %.oxx,%.debug.oxx,$(OBJS_CXX))

# Release/debug build rules
$(BUILD_DIR)/$(PROJECT_NAME) build: $(OBJS_C) $(OBJS_CXX)
	$(CXX) -o $(BUILD_DIR)/$(PROJECT_NAME) $^

$(BUILD_DIR)/$(PROJECT_NAME).debug build_debug: $(DEBUG_OBJS_C) $(DEBUG_OBJS_CXX)
	$(CXX) -o $(BUILD_DIR)/$(PROJECT_NAME).debug $^


.PHONY: build build_debug run debug clean memleak compile_commands

run: $(BUILD_DIR)/$(PROJECT_NAME)
	$(BUILD_DIR)/$(PROJECT_NAME)

debug: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(DEBUGGER) $(BUILD_DIR)/$(PROJECT_NAME).debug

memleak: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(MEMLEAK_TESTER) $(BUILD_DIR)/$(PROJECT_NAME).debug

clean:
	rm -rf $(BUILD_DIR)

# Exports compile commands
compile_commands: clean
	bear -- $(MAKE) build_debug

# Directory creation
.SECONDARY:
$(OBJ_DIR)/%.d: | $(BUILD_DIR)
	mkdir -p $(dir $@)
	touch $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
