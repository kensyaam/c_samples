##########################################
# ディレクトリ構成
##########################################
# メインプログラム用
SRC_DIR        = src
OBJ_DIR        = build/obj
BIN_DIR        = build/bin

# ライブラリ用
LIB_SRC_DIR    = src_lib
LIB_OBJ_DIR    = build/lib_obj
LIB_DIR        = build/lib

# UT用
UT_FW_DIR      = test/fw
UT_SRC_DIR     = test/tests
UT_OBJ_DIR     = build/test_obj
UT_BIN_DIR     = build/test_bin
UT_REPORT_DIR  = build/report

# スタブ用
STUB_SRC_DIR   = test/stub
STUB_OBJ_DIR   = build/stub_obj

##########################################
# ビルド用の変数
##########################################
# コンパイラ
CC = gcc
CXX = g++

ifeq ($(DEV), 1)
# サニタイザフラグ
#    dnf install libasan libubsan しておく必要あり
SANITIZE_FLAGS = -fsanitize=undefined,address
else
SANITIZE_FLAGS =
endif

# コンパイルオプション (-I<dir>で検索場所追加)
CFLAGS = -Wall -Wextra -O0 $(SANITIZE_FLAGS)
# 自作ライブラリを使う場合
# CFLAGS += -I$(LIB_SRC_DIR)

# ライブラリリンクオプション (-L<dir>で検索場所追加)
#   「-Wl,-rpath,'$$(ORIGIN)'/build/lib」を付けない場合は、実行時にLD_LIBRARY_PATHを設定する必要がある
LDFLAGS = -lpthread $(SANITIZE_FLAGS)
# 自作ライブラリを使う場合
# LDFLAGS += -lsample -L$(LIB_DIR) -Wl,-rpath,'$$(ORIGIN)'/$(LIB_DIR)

# echo用のエスケープシーケンス
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
RESET = \033[0m

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
EXE_WITH_STUB = $(EXE)_with_stub

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
# ソースファイル
UT_SRCS = $(wildcard $(UT_SRC_DIR)/*.cpp)
# オブジェクトファイル
UT_OBJS = $(patsubst $(UT_SRC_DIR)/%.cpp, $(UT_OBJ_DIR)/%.o, $(UT_SRCS))
# 実行ファイルのメインオブジェクトファイル (UT_OBJS から test_*.o のみ抽出したもの)
UT_TARGET_OBJS = $(filter $(UT_OBJ_DIR)/test_%.o, $(UT_OBJS))
# ユーティリティのオブジェクトファイル (UT_OBJS から test_*.o を除いたもの)
UT_UTIL_OBJS = $(filter-out $(UT_TARGET_OBJS), $(UT_OBJS))

# 実行ファイル
UT_EXES = $(patsubst $(UT_OBJ_DIR)/test_%.o, $(UT_BIN_DIR)/test_%, $(UT_TARGET_OBJS))

# コンパイルオプション
UT_CFLAGS = -coverage -g -O0 -DUNIT_TEST $(SANITIZE_FLAGS)
# リンクオプション
UT_LDFLAGS = -lpthread -lcurl -lgtest -lgtest_main -L$(UT_FW_DIR)/lib64 -Wl,-rpath,'$$(ORIGIN)'/$(UT_FW_DIR)/lib64 -lgcov $(SANITIZE_FLAGS)

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
.PHONY: all

# ---------------------------------------------------
# メインプログラム用
# ---------------------------------------------------
# 実行ファイルの生成
$(EXE): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo -e "$(GREEN)$@ を生成しました。$(RESET)"

# オブジェクトファイルの生成
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# スタンドアローン用の実行ファイルの生成
$(EXE_WITH_STUB): $(OBJS) $(STUB_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo -e "$(GREEN)$@ を生成しました。$(RESET)"

# ---------------------------------------------------
# 共有ライブラリ用
# ---------------------------------------------------
# 共有ライブラリの生成
lib: $(SHARED_LIB)
.PHONY: lib

$(SHARED_LIB): $(LIB_OBJS)
	@mkdir -p $(LIB_DIR)
	$(CC) -shared -o $@ $^
	@echo -e "$(GREEN)$@ を生成しました。$(RESET)"

# 共有ライブラリ用のオブジェクトファイルの生成
$(LIB_OBJ_DIR)/%.o: $(LIB_SRC_DIR)/%.c
	@mkdir -p $(LIB_OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# ---------------------------------------------------
# UT用
# ---------------------------------------------------
test: $(UT_EXES)
	@mkdir -p $(UT_REPORT_DIR)
	@for exe in $(UT_EXES); do \
		exe_name=$$(basename $$exe); \
		echo -e "$(YELLOW)----------------------------------------$(RESET)"; \
		echo -e "$(YELLOW)exec $$exe $(RESET)"; \
		echo -e "$(YELLOW)----------------------------------------$(RESET)"; \
		$$exe --gtest_output=json:$(UT_REPORT_DIR)/$$exe_name.json; \
		python test/tool/gtest_json_to_csv.py $(UT_REPORT_DIR)/$$exe_name.json $(UT_REPORT_DIR)/$$exe_name tsv; \
		echo ""; \
	done
	@echo "カバレッジ情報 ----------------------"
	@lcov -q -d $(UT_OBJ_DIR) -c --rc lcov_branch_coverage=1 -o $(UT_REPORT_DIR)/lcov.info
	@lcov -q --remove $(UT_REPORT_DIR)/lcov.info '/usr/*' '*/gtest/*' '*/test/tests/*' --rc lcov_branch_coverage=1 -o $(UT_REPORT_DIR)/CoreCoverage.info
	@lcov -q --list $(UT_REPORT_DIR)/CoreCoverage.info --rc lcov_branch_coverage=1
	@genhtml -q $(UT_REPORT_DIR)/CoreCoverage.info --branch-coverage -o $(UT_REPORT_DIR)/coverage
	@echo "カバレッジ情報(html)を出力しました : $(UT_REPORT_DIR)/coverage/index.html"
.PHONY: test

test_%: $(UT_BIN_DIR)/test_%
	@mkdir -p $(UT_REPORT_DIR)
	@exe_name=$$(basename $<); \
		echo -e "$(YELLOW)----------------------------------------$(RESET)"; \
		echo -e "$(YELLOW)exec $< $(RESET)"; \
		echo -e "$(YELLOW)----------------------------------------$(RESET)"; \
		$< --gtest_output=json:$(UT_REPORT_DIR)/$<_name.json; \
		python test/tool/gtest_json_to_csv.py $(UT_REPORT_DIR)/$$exe_name.json $(UT_REPORT_DIR)/$<_name tsv; \
		echo "";
	@echo "カバレッジ情報 ----------------------"
	@lcov -q -d $(UT_OBJ_DIR) -c --rc lcov_branch_coverage=1 -o $(UT_REPORT_DIR)/lcov.info
	@lcov -q --remove $(UT_REPORT_DIR)/lcov.info '/usr/*' '*/gtest/*' '*/test/tests/*' --rc lcov_branch_coverage=1 -o $(UT_REPORT_DIR)/CoreCoverage.info
	@lcov -q --list $(UT_REPORT_DIR)/CoreCoverage.info --rc lcov_branch_coverage=1
	@genhtml -q $(UT_REPORT_DIR)/CoreCoverage.info --branch-coverage -o $(UT_REPORT_DIR)/coverage
	@echo "カバレッジ情報(html)を出力しました : $(UT_REPORT_DIR)/coverage/index.html"
.PHONY: test_%

test_build: $(UT_EXES)
.PHONY: test_build

test_list: $(UT_EXES)
	@echo "UT_OBJS: $(UT_OBJS)" | sed -E 's/ +/\n/g'
	@echo "UT_TARGET_OBJS: $(UT_TARGET_OBJS)" | sed -E 's/ +/\n/g'
	@echo "UT_UTIL_OBJS: $(UT_UTIL_OBJS)" | sed -E 's/ +/\n/g'
	@echo "UT_EXES: $(UT_EXES)" | sed -E 's/ +/\n/g'
	@for exe in $(UT_EXES); do \
		exe_name=$$(basename $$exe); \
		echo "$$exe_name"; \
		$$exe --gtest_list_tests; \
	done
.PHONY: test_list

# UT用の実行ファイルの生成
$(UT_BIN_DIR)/test_%: $(UT_OBJ_DIR)/test_%.o $(UT_UTIL_OBJS)
	@mkdir -p $(UT_BIN_DIR)
	$(CXX) -o $@ $^ $(UT_LDFLAGS) || true
	@if [[ -f $@ ]]; then \
		echo -e "$(GREEN)$@ を生成しました。$(RESET)"; \
	else \
		echo -e "$(RED)$@ の生成に失敗しました。$(RESET)"; false; \
	fi

# UT用のオブジェクトファイルの生成
$(UT_OBJ_DIR)/%.o: $(UT_SRC_DIR)/%.cpp 
	@if [[ ! -f $(UT_FW_DIR)/lib64/libgtest.a ]]; then \
		echo -e "$(RED)GoogleTestがセットアップされていません。make setup_testを実行してください。$(RESET)"; \
		exit 1; \
	fi
	@mkdir -p $(UT_OBJ_DIR)
	$(CXX) $(UT_CFLAGS) -c $< -o $@ -I$(UT_FW_DIR)/include -I$(SRC_DIR) -I$(LIB_SRC_DIR)

# GoogleTest、FFFをセットアップ
setup_test:
	@rm -rf $(UT_FW_DIR)/*
	@mkdir -p $(UT_FW_DIR)
	@cd $(UT_FW_DIR) && \
		curl -OL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz && \
		tar -zxvf googletest-1.16.0.tar.gz && \
		rm googletest-1.16.0.tar.gz && \
		mkdir -p googletest-1.16.0/build && \
		cd googletest-1.16.0/build && \
		cmake -DCMAKE_INSTALL_PREFIX=../.. .. && \
		make && make install && \
		cd ../.. && rm -rf googletest-1.16.0
	@cd $(UT_FW_DIR) && \
		curl -L https://github.com/meekrosoft/fff/releases/download/v1.1/fff.h -o include/fff.h
	@echo -e "$(GREEN)GoogleTestとFFFのセットアップが完了しました。$(RESET)"
.PHONY: setup_test

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
	rm -rf ./build/*
.PHONY: clean
