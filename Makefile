CC = gcc
CFLAGS = -Wall -g
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin

MANAGER_SRC = $(SRC_DIR)/nfs_manager.c
CONSOLE_SRC = $(SRC_DIR)/nfs_console.c

MANAGER_BIN = $(BIN_DIR)/nfs_manager
CONSOLE_BIN = $(BIN_DIR)/nfs_console

all: $(MANAGER_BIN) $(CONSOLE_BIN)

$(MANAGER_BIN): $(MANAGER_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(CONSOLE_BIN): $(CONSOLE_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
