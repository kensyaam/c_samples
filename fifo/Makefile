OBJ_DIR = build/obj
BIN_DIR = build/bin
LIB_DIR = build/lib

# コンパイラ
CC = gcc

# コンパイルオプション
CFLAGS = -Wall -Wextra -O2 -Iinclude -Ilib

# ライブラリリンクオプション (-L<dir>で検索場所追加)
#   「-Wl,-rpath=build/lib」を付けない場合は、実行時にLD_LIBRARY_PATHを設定する必要がある
LDFLAGS = -lcommon -L$(LIB_DIR) -Wl,-rpath=build/lib

# ターゲット実行ファイル
EXE_SENDER = $(BIN_DIR)/sender
EXE_RECEIVER = $(BIN_DIR)/receiver
# ターゲットライブラリ
SHARED_LIB = $(LIB_DIR)/libcommon.so

# ソースファイル
SRCS += $(wildcard src/*.c)
LIB_SRCS = $(wildcard lib/*.c)

# オブジェクトファイル
OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRCS))
LIB_OBJS = $(patsubst lib/%.c, $(OBJ_DIR)/lib/%.o, $(LIB_SRCS))

# デフォルトターゲット
all: $(EXE_SENDER) $(EXE_RECEIVER) $(SHARED_LIB)

# 実行ファイルの生成
$(EXE_SENDER): $(OBJ_DIR)/sender.o $(SHARED_LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter-out $(SHARED_LIB),$^) $(LDFLAGS)

$(EXE_RECEIVER): $(OBJ_DIR)/receiver.o $(SHARED_LIB)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(filter-out $(SHARED_LIB),$^) $(LDFLAGS)

# オブジェクトファイルの生成
#%.o: %.c
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

lib: $(SHARED_LIB)

# 共有ライブラリの生成
$(SHARED_LIB): $(LIB_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $^

# 共有ライブラリ用のオブジェクトファイルの生成
$(OBJ_DIR)/lib/%.o: lib/%.c
	@mkdir -p $(OBJ_DIR)/lib
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS) $(EXE_SENDER) $(EXE_RECEIVER) $(LIB_OBJS) $(SHARED_LIB)

# .PHONYターゲット
.PHONY: all clean lib
