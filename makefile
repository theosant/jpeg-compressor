# Configurações básicas
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)
LDFLAGS = -mconsole -lm
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2

# Diretórios
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)\obj

# Arquivos fonte e objetos
SRCS = $(wildcard $(SRC_DIR)/*.c) ./main.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(wildcard $(INC_DIR)/*.h)

# Nome do executável
TARGET = $(BIN_DIR)\main.exe

# Regra principal (release por padrão)
all: release

# Build de release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

# Build para debug
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Link do executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Compilação dos objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Criação dos diretórios necessários (compatível com Windows)
$(BIN_DIR) $(OBJ_DIR):
	if not exist "$@" mkdir "$@"

# Execução
run: release
	$(TARGET)

# Limpeza (compatível com Windows)
clean:
	if exist "$(BIN_DIR)" rmdir /s /q "$(BIN_DIR)"

# Informações do projeto
info:
	@echo Sources: $(SRCS)
	@echo Objects: $(OBJS)
	@echo Dependencies: $(DEPS)

.PHONY: all release debug run clean info