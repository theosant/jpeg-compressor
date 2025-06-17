# Detectar sistema operacional
ifeq ($(OS),Windows_NT)
    RM = if exist $(1) rmdir /s /q $(1)
    MKDIR = if not exist $(1) mkdir $(1)
    EXEC = $(TARGET).exe
    SEP = \\
else
    RM = rm -rf $(1)
    MKDIR = mkdir -p $(1)
    EXEC = ./$(TARGET)
    SEP = /
endif

# Diretórios
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)$(SEP)obj

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)
LDFLAGS = -lm

DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2

# Arquivos
SRCS = $(wildcard $(SRC_DIR)/*.c) main.c
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(wildcard $(INC_DIR)/*.h)

# Nome do executável (sem extensão aqui)
TARGET = $(BIN_DIR)$(SEP)main

# Regra padrão
all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(EXEC)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(EXEC)

# Linkagem
$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Compilar objetos
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Criação de diretórios
$(BIN_DIR):
	$(call MKDIR,$@)

$(OBJ_DIR):
	$(call MKDIR,$@)

# Execução
run: release
	$(EXEC)

# Limpeza
clean:
	$(call RM,$(BIN_DIR))

# Info
info:
	@echo OS: $(OS)
	@echo Executável: $(EXEC)
	@echo Fontes: $(SRCS)
	@echo Objetos: $(OBJS)

.PHONY: all release debug run clean info
