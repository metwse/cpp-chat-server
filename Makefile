PROJECT_NAME = chatd

CC = gcc
CXX = g++
DEBUGGER = gdb
MEMCHECK = valgrind

C_FLAGS=-O2 -Wall -Werror -D_GNU_SOURCE -std=gnu17 -I.
T_FLAGS=-O0 -g3 -Wall -std=gnu17 -D_DEBUG -D_GNU_SOURCE -I.

CXX_FLAGS=-O2 -Wall -Werror -std=c++17 -I.
TXX_FLAGS=-O0 -g3 -Wall -std=c++17 -D_DEBUG -I.

# Recursive wildcard search
rwildcard = $(wildcard $1$2) \
	    $(foreach d, \
		    $(wildcard $1*), \
		    $(call rwildcard,$d/,$2))
# Removes elements of array that contain given string
rfilter-out = $(foreach v,$(2),$(if $(findstring $(1),$(v)),,$(v)))

SRC_DIR = $(PROJECT_NAME)
BUILD_DIR = target
OBJ_DIR = $(BUILD_DIR)/obj

SRC_C = $(filter-out %.test.c,$(call rwildcard,$(SRC_DIR)/,*.c))
SRC_CXX = $(filter-out %.test.cpp,$(call rwildcard,$(SRC_DIR)/,*.cpp))


# Pattern rules for object files
# ---------------------------------------------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/%.d
	$(CC) -c $(C_FLAGS) $< -o $@

$(OBJ_DIR)/%.oxx: $(SRC_DIR)/%.cpp | $(OBJ_DIR)/%.d
	$(CXX) -c $(CXX_FLAGS) $< -o $@

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)/%.d
	$(CC) -c $(T_FLAGS) $< -o $@

$(OBJ_DIR)/%.debug.oxx: $(SRC_DIR)/%.cpp | $(OBJ_DIR)/%.d
	$(CXX) -c $(TXX_FLAGS) $< -o $@
# ===========================================================================


# Object files that will included to final executable
OBJS_C = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_C))
DEBUG_OBJS_C = $(patsubst %.o,%.debug.o,$(OBJS_C))
OBJS_CXX = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.oxx,$(SRC_CXX))
DEBUG_OBJS_CXX = $(patsubst %.oxx,%.debug.oxx,$(OBJS_CXX))

# Object files except main. Used in testing
NONMAIN_C_DEBUG_OBJS = $(call rfilter-out,main,$(DEBUG_OBJS_C))
NONMAIN_CXX_DEBUG_OBJS = $(call rfilter-out,main,$(DEBUG_OBJS_CXX))


# Release/debug build rules
# ---------------------------------------------------------------------------
$(BUILD_DIR)/$(PROJECT_NAME): $(OBJS_C) $(OBJS_CXX)
	$(CXX) -o $@ \
		$^

$(BUILD_DIR)/$(PROJECT_NAME).debug: $(DEBUG_OBJS_C) $(DEBUG_OBJS_CXX)
	$(CXX) -o $@ \
		$^

$(BUILD_DIR)/tests/%.xx.test: $(SRC_DIR)/%.test.cpp \
	$(NONMAIN_C_DEBUG_OBJS) $(NONMAIN_CXX_DEBUG_OBJS) \
	| $(BUILD_DIR)/tests/%.d

	$(CXX) $(TXX_FLAGS) \
		-o $@ \
		$^

$(BUILD_DIR)/tests/%.test: $(SRC_DIR)/%.test.c \
	$(NONMAIN_C_DEBUG_OBJS) | $(BUILD_DIR)/tests/%.d

	$(CC) $(T_FLAGS) \
		-o $@ \
		$^

.PHONY += run debug clean memleak \
	  build build_debug \
	  compile_commands
# ===========================================================================


# Shorthand build/run rules
# ---------------------------------------------------------------------------
build: $(BUILD_DIR)/$(PROJECT_NAME)
	@echo $@

build_debug: $(BUILD_DIR)/$(PROJECT_NAME).debug
	@echo $@

run: $(BUILD_DIR)/$(PROJECT_NAME)
	$^

debug: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(DEBUGGER) $^


TEST_SRC_C = $(call rwildcard,$(SRC_DIR)/,*.test.c)
TEST_SRC_CXX = $(call rwildcard,$(SRC_DIR)/,*.test.cpp)

# Shorthand rules for test builds
define test_rules
build_test_$1: $$(BUILD_DIR)/tests/$1.test
	@echo $$@

memcheck_test_$1: $$(BUILD_DIR)/tests/$1.test
	$$(MEMCHECK) $$^

test_$1: $$(BUILD_DIR)/tests/$1.test
	$$(DEBUGGER) $$^

.PHONY += test_$1 build_test_$1 memcheck_test_$1
endef

CXX_TESTS = $(patsubst $(SRC_DIR)/%.test.cpp,\
	    $(BUILD_DIR)/tests/%.xx.test,$(TEST_SRC_CXX))
C_TESTS = $(patsubst $(SRC_DIR)/%.test.c,\
	  $(BUILD_DIR)/tests/%.test,$(TEST_SRC_C))

$(foreach f, \
	$(patsubst $(SRC_DIR)/%.test.c,%,$(TEST_SRC_C)), \
	$(eval $(call test_rules,$(f))))

$(foreach f, \
	$(patsubst $(SRC_DIR)/%.test.cpp,%.xx,$(TEST_SRC_CXX)), \
	$(eval $(call test_rules,$(f))))

test_all: $(C_TESTS) $(CXX_TESTS)
	for test in $^; do \
		echo "Running $$test"; \
		$$test; \
	done

build_test_all: $(C_TESTS) $(CXX_TESTS)
	@echo "" $(foreach test,$^,"$(test)\n")

memcheck_test_all: $(C_TESTS) $(CXX_TESTS)
	for test in $^; do \
		$(MEMCHECK) $$test; \
	done

.PHONY += test_all build_test_all memcheck_test_all

# ===========================================================================

memcheck: $(BUILD_DIR)/$(PROJECT_NAME).debug
	$(MEMCHECK) $(BUILD_DIR)/$(PROJECT_NAME).debug

clean:
	rm -rf $(BUILD_DIR)

# Exports compile_commands.json
compile_commands: clean
	bear -- $(MAKE) build_debug

# Directory creation
.SECONDARY:
$(BUILD_DIR)/%.d: | $(BUILD_DIR)
	mkdir -p $(dir $@)
	touch $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
