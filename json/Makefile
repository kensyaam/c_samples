# コンパイラ
CC = gcc

# コンパイルオプション
CFLAGS = -Wall -Wextra -O2 -Iinclude

# ライブラリリンクオプション (-L<dir>で検索場所追加)
LDFLAGS = -ljson-c

OBJ_DIR = build/obj
BIN_DIR = build/bin

# ターゲット実行ファイル
EXE = $(BIN_DIR)/sample

# ソースファイル
SRCS += $(wildcard src/*.c)

# オブジェクトファイル
#OBJS = $(SRCS:.c=.o)
OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# デフォルトターゲット
all: $(EXE)

# 実行ファイルの生成
$(EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# オブジェクトファイルの生成
#%.o: %.c
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS) $(EXE)

# テストの実行
test: $(EXE)
	@for test in tests/*.c; do \
	    $(CC) $(CFLAGS) $$test -o $${test%.c}.out && $${test%.c}.out; \
	done

# .PHONYターゲット
.PHONY: all clean test