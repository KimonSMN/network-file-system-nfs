CC = gcc
CFLAGS = -Wall -g -lpthread
INCLUDES = -Iinclude

SRC_DIR = src
BIN_DIR = bin

MANAGER_SRC = $(SRC_DIR)/nfs_manager.c
CONSOLE_SRC = $(SRC_DIR)/nfs_console.c
CLIENT_SRC = $(SRC_DIR)/nfs_client.c

COMMON_SRCS = $(SRC_DIR)/queue.c $(SRC_DIR)/sync_info_mem_store.c
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

MANAGER_BIN = $(BIN_DIR)/nfs_manager
CONSOLE_BIN = $(BIN_DIR)/nfs_console
CLIENT_BIN = $(BIN_DIR)/nfs_client

all: $(MANAGER_BIN) $(CONSOLE_BIN) $(CLIENT_BIN)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(MANAGER_BIN): $(MANAGER_SRC) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^
	
$(CONSOLE_BIN): $(CONSOLE_SRC) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(CLIENT_BIN): $(CLIENT_SRC) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
