# Diretórios
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)

# Arquivos fonte
SRCS = main.c $(wildcard $(SRC_DIR)/*.c)

# Nome do executável
TARGET = $(BIN_DIR)/main

# Regra principal
all: $(TARGET)

# Compilação do executável
$(TARGET): $(BIN_DIR) $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Criação da pasta bin, se necessário
$(BIN_DIR):
	mkdir -p $(BIN_DIR)


run: all
	./$(TARGET)
	
# Limpeza
clean:
	rm -rf $(BIN_DIR)
