PROJECT_NAME = chatd

CC = gcc
CXX = g++
DEBUGGER = gdb
MEMLEAK_TESTER = valgrind

C_FLAGS=-O2 -Wall -Werror -D_GNU_SOURCE -std=gnu17 -I.
T_FLAGS=-O0 -g3 -Wall -std=gnu17 -D_DEBUG -D_GNU_SOURCE -I.

CXX_FLAGS=-O2 -Wall -Werror -std=c++17 -I.
TXX_FLAGS=-O0 -g3 -Wall -std=c++17 -D_DEBUG -I.

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
filter_out = $(foreach v,$(2),$(if $(findstring $(1),$(v)),,$(v)))

SRC_DIR = $(PROJECT_NAME)
BUILD_DIR = target
OBJ_DIR = $(BUILD_DIR)/obj

SRC_C = $(filter-out %.test.c,$(call rwildcard,$(SRC_DIR)/,*.c))
SRC_CXX = $(filter-out %.test.cpp,$(call rwildcard,$(SRC_DIR)/,*.cpp))


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

NONMAIN_C_DEBUG_OBJS = $(call filter_out,main,$(DEBUG_OBJS_C))
NONMAIN_CXX_DEBUG_OBJS = $(call filter_out,main,$(DEBUG_OBJS_C))


# Release/debug build rules
$(BUILD_DIR)/$(PROJECT_NAME): $(OBJS_C) $(OBJS_CXX)
	$(CXX) -o $@ $^

$(BUILD_DIR)/$(PROJECT_NAME).debug: $(DEBUG_OBJS_C) $(DEBUG_OBJS_CXX)
	$(CXX) -o $@ $^

$(BUILD_DIR)/tests/%.test: $(NONMAIN_C_DEBUG_OBJS) | $(BUILD_DIR)/tests/%.d
	$(CC) -o $@ $^ \
		$(patsubst $(BUILD_DIR)/tests/%.test,$(SRC_DIR)/%.test.c,$@)

$(BUILD_DIR)/tests/%.testxx: $(NONMAIN_C_DEBUG_OBJS) $(NONMAIN_CXX_DEBUG_OBJS) | $(BUILD_DIR)/tests/%.d
	$(CXX) -o $@ $^ \
		$(patsubst $(BUILD_DIR)/tests/%.testxx,$(SRC_DIR)/%.test.cpp,$@)

.PHONY: run debug clean memleak test_% \
	build build_debug build_test_% \
	compile_commands


# Binary build rules
build: $(BUILD_DIR)/$(PROJECT_NAME)
	@echo $@

build_debug: $(BUILD_DIR)/$(PROJECT_NAME).debug
	@echo $@

build_test_%xx: $(BUILD_DIR)/tests/%.testxx
	@echo $@

build_test_%: $(BUILD_DIR)/tests/%.test
	@echo $@


# Binary execution rules
run: $(BUILD_DIR)/$(PROJECT_NAME)
	$^

debug: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(DEBUGGER) $^

test_%xx: $(BUILD_DIR)/tests/%.testxx
	$(DEBUGGER) $^

test_%: $(BUILD_DIR)/tests/%.test
	$(DEBUGGER) $^


memleak: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(MEMLEAK_TESTER) $(BUILD_DIR)/$(PROJECT_NAME).debug

clean:
	rm -rf $(BUILD_DIR)

# Exports compile commands
compile_commands: clean
	bear -- $(MAKE) build_debug

# Directory creation
.SECONDARY:
$(BUILD_DIR)/%.d: | $(BUILD_DIR)
	mkdir -p $(dir $@)
	touch $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
