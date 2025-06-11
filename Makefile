CC := gcc
AS := nasm

PROGRAM := main

BUILD_DIR := ./build
SRC_DIRS := ./src
OUTPUT_DIR := .

EXECUTABLE := $(OUTPUT_DIR)/$(PROGRAM)

SRCS := $(shell find $(SRC_DIRS) -name '*.c' -or -name '*.s')

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += cglm/include/
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

ASFLAGS := -felf64

CPPFLAGS := $(INC_FLAGS) -MMD -MP
CCFLAGS := -Wall -Werror -Wextra -DNDEBUG

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

all: $(EXECUTABLE)

debug: CCFLAGS += -UNDEBUG -g
debug: clean $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.s.o: %.s
	mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

.PHONY: test clean compile_commands
test: $(EXECUTABLE)
	$(EXECUTABLE)

clean:
	rm -r $(BUILD_DIR)
	rm $(EXECUTABLE)

# Generates compile_commands.json for clangd lsp server
# Requires package `compiledb`, run whenever editing compiler flags
compile_commands:
	make clean
	compiledb make

-include $(DEPS)
