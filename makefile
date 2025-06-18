# Detectar sistema operacional
ifeq ($(OS),Windows_NT)
    RM = if exist $(1) rmdir /s /q $(1)
    MKDIR = if not exist $(1) mkdir $(1)
    SEP = \\
else
    RM = rm -rf $(1)
    MKDIR = mkdir -p $(1)
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

# Arquivos comuns
COMMON_SRCS = $(wildcard $(SRC_DIR)/*.c)
COMMON_OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(COMMON_SRCS))
DEPS = $(wildcard $(INC_DIR)/*.h)

# Alvos
COMPRESSOR = $(BIN_DIR)$(SEP)compressor
DECOMPRESSOR = $(BIN_DIR)$(SEP)descompressor

COMPRESSOR_OBJ = $(OBJ_DIR)/compressor.o
DECOMPRESSOR_OBJ = $(OBJ_DIR)/descompressor.o

# Regra padrão
all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(COMPRESSOR) $(DECOMPRESSOR)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(COMPRESSOR) $(DECOMPRESSOR)

# Compilação dos executáveis
$(COMPRESSOR): $(COMPRESSOR_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(DECOMPRESSOR): $(DECOMPRESSOR_OBJ) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# Regras para os .o principais (fora de src/)
$(OBJ_DIR)/compressor.o: compressor.c $(DEPS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ compressor.c

$(OBJ_DIR)/descompressor.o: descompressor.c $(DEPS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ descompressor.c

# Compilação dos objetos comuns (dentro de src/)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Criação de diretórios
$(BIN_DIR):
	$(call MKDIR,$@)

$(OBJ_DIR):
	$(call MKDIR,$@)

# Execuções manuais
run-compressor: release
	$(COMPRESSOR)

run-descompressor: release
	$(DECOMPRESSOR)

# Limpeza
clean:
	$(call RM,$(BIN_DIR))

# Info
info:
	@echo OS: $(OS)
	@echo Compressor: $(COMPRESSOR)
	@echo Descompressor: $(DECOMPRESSOR)
	@echo Fontes comuns: $(COMMON_SRCS)
	@echo Objetos comuns: $(COMMON_OBJS)

.PHONY: all release debug clean run-compressor run-descompressor info
