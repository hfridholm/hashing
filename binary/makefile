COMPILER := gcc
COMPILE_FLAGS := -Wall -Werror -g -O0 -std=gnu99 -oFast

SOURCE_DIR := ../source
OBJECT_DIR := ../object
BINARY_DIR := ../binary

SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.c)
HEADER_FILES := $(wildcard $(SOURCE_DIR)/*.h)

OBJECT_FILES := $(addprefix $(OBJECT_DIR)/, $(notdir $(SOURCE_FILES:.c=.o)))

default: hashing

hashing: $(OBJECT_FILES) $(SOURCE_FILES) $(HEADER_FILES)
	$(COMPILER) $(OBJECT_FILES) -o $(BINARY_DIR)/hashing

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(COMPILER) $< -c $(COMPILE_FLAGS) -o $@

.PRECIOUS: $(OBJECT_DIR)/%.o hashing

clean:
	-rm 2>/dev/null $(OBJECT_DIR)/*.o hashing
