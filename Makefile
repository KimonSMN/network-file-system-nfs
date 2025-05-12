CC = gcc
CFLAGS = -Wall -g
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin

MANAGER_SRC = $(SRC_DIR)/nfs_manager.c

MANAGER_BIN = $(BIN_DIR)/nfs_manager

all: $(MANAGER_BIN)

$(MANAGER_BIN): $(MANAGER_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up
clean:
	rm -rf $(BIN_DIR)

# Phony targets
.PHONY: all clean