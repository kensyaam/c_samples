##########################################
# ディレクトリ構成
##########################################
# メインプログラム用
SRC_DIR        = src
OBJ_DIR        = build/obj
BIN_DIR        = build/bin

# ライブラリ用
LIB_SRC_DIR    = src_lib
LIB_OBJ_DIR    = build/obj_lib
LIB_DIR        = build/lib

# UT用
UT_FW_DIR      = test/fw
UT_SRC_DIR     = test/tests
UT_OBJ_DIR     = build/obj_test
UT_REPORT_DIR  = build/report

# スタブ用
STUB_SRC_DIR   = test/stub
STUB_OBJ_DIR   = build/obj_stub

##########################################
# ビルド用の変数
##########################################
# コンパイラ
CC = gcc
CXX = g++

# コンパイルオプション (-I<dir>で検索場所追加)
CFLAGS = -Wall -Wextra -O2 -I$(LIB_SRC_DIR)

# ライブラリリンクオプション (-L<dir>で検索場所追加)
#   「-Wl,-rpath=build/lib」を付けない場合は、実行時にLD_LIBRARY_PATHを設定する必要がある
LDFLAGS = -lpthread -lsample -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR)

# ---------------------------------------------------
# メインプログラム用
# ---------------------------------------------------
# 実行ファイル
EXE = $(BIN_DIR)/sample
# ソースファイル
SRCS += $(wildcard $(SRC_DIR)/*.c)
# オブジェクトファイル
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# 実行ファイル（スタンドアローン用）
EXE_WITH_STUB = $(BIN_DIR)/sample_with_stub

# ---------------------------------------------------
# 共有ライブラリ用
# ---------------------------------------------------
# 共有ライブラリファイル
SHARED_LIB = $(LIB_DIR)/libsample.so
# ソースファイル
LIB_SRCS = $(wildcard $(LIB_SRC_DIR)/*.c)
# オブジェクトファイル
LIB_OBJS = $(patsubst $(LIB_SRC_DIR)/%.c, $(LIB_OBJ_DIR)/%.o, $(LIB_SRCS))

# ---------------------------------------------------
# UT用
# ---------------------------------------------------
# 実行ファイル
UT_EXE = $(BIN_DIR)/test
# ソースファイル
UT_SRCS = $(wildcard $(UT_SRC_DIR)/*.cpp)
# オブジェクトファイル
UT_OBJS = $(patsubst $(UT_SRC_DIR)/%.cpp, $(UT_OBJ_DIR)/%.o, $(UT_SRCS))
UT_OBJS += $(wildcard $(UT_FW_DIR)/gtest/obj/*.o)

# ---------------------------------------------------
# スタブ用
# ---------------------------------------------------
# ソースファイル
STUB_SRCS = $(wildcard $(STUB_SRC_DIR)/*.c)
# オブジェクトファイル
STUB_OBJS = $(patsubst $(STUB_SRC_DIR)/%.c, $(STUB_OBJ_DIR)/%.o, $(STUB_SRCS))


##########################################
# makeターゲット
##########################################
# デフォルトターゲット
#all: $(EXE) $(SHARED_LIB)
all: $(EXE)

# ---------------------------------------------------
# メインプログラム用
# ---------------------------------------------------
# 実行ファイルの生成
$(EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# オブジェクトファイルの生成
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# スタンドアローン用の実行ファイルの生成
$(EXE_WITH_STUB): $(OBJS) $(STUB_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ---------------------------------------------------
# 共有ライブラリ用
# ---------------------------------------------------
# 共有ライブラリの生成
lib: $(SHARED_LIB)

$(SHARED_LIB): $(LIB_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $^

# 共有ライブラリ用のオブジェクトファイルの生成
$(LIB_OBJ_DIR)/%.o: $(LIB_SRC_DIR)/%.c
	@mkdir -p $(LIB_OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# ---------------------------------------------------
# UT用
# ---------------------------------------------------
test: $(UT_EXE)
	@mkdir -p $(UT_REPORT_DIR)
	-./$(UT_EXE) --gtest_output=json:$(UT_REPORT_DIR)/test_report.json
	python test/tool/gtest_json_to_csv.py $(UT_REPORT_DIR)/test_report.json $(UT_REPORT_DIR)/test_report.tsv tsv

test_list: $(UT_EXE)
	./$(UT_EXE) --gtest_list_tests

# UT用の実行ファイルの生成
$(UT_EXE): $(UT_OBJS) $(OBJS)
	@if [[ ! -f $(UT_FW_DIR)/gtest/gtest-all.cc ]]; then \
		echo "GoogleTestがセットアップされていません。make setup_testを実行してください。"; \
		exit 1; \
	fi
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $(filter-out $(OBJ_DIR)/main.o,$^) \
		-I$(UT_FW_DIR) -lpthread $(LDFLAGS) 

# UT用のオブジェクトファイルの生成
$(UT_OBJ_DIR)/%.o: $(UT_SRC_DIR)/%.cpp 
	@mkdir -p $(UT_OBJ_DIR)
	$(CXX) -c $< -o $@ -DUNIT_TEST -I$(UT_FW_DIR) -I$(SRC_DIR) -I$(LIB_SRC_DIR)

# $(UT_FW_DIR)/gtest/gtest.o: $(UT_FW_DIR)/gtest/gtest-all.cc $(UT_FW_DIR)/googletest-release-1.11.0/googletest/src/gtest_main.cc
# 	$(CXX) -c $^ -o $@ -I$(UT_FW_DIR)

# GoogleTest、FFFをセットアップ
setup_test:
	@rm -rf $(UT_FW_DIR)/*
	@mkdir -p $(UT_FW_DIR)
	curl -OL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
	unzip -o release-1.11.0.zip
	rm release-1.11.0.zip
	python googletest-release-1.11.0/googletest/scripts/fuse_gtest_files.py $(UT_FW_DIR)
	mv -f googletest-release-1.11.0 $(UT_FW_DIR)/.
	@mkdir -p $(UT_FW_DIR)/gtest/obj
	$(CXX) -c $(UT_FW_DIR)/gtest/gtest-all.cc -o $(UT_FW_DIR)/gtest/obj/gtest-all.o -I$(UT_FW_DIR)
	$(CXX) -c $(UT_FW_DIR)/googletest-release-1.11.0/googletest/src/gtest_main.cc -o $(UT_FW_DIR)/gtest/obj/gtest_main.o -I$(UT_FW_DIR)
	curl -OL https://github.com/meekrosoft/fff/releases/download/v1.1/fff.h
	mv -f fff.h $(UT_FW_DIR)/.

# ---------------------------------------------------
# スタブ用
# ---------------------------------------------------
# スタブ用のオブジェクトファイルの生成
$(STUB_OBJ_DIR)/%.o: $(STUB_SRC_DIR)/%.c
	@mkdir -p $(STUB_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------------------------------------------------
# クリーンアップ
clean:
	find ./build -type f -delete

# .PHONYターゲット
.PHONY: all clean lib setup_test test test_list
