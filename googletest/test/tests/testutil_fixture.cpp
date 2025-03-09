extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "testutil.hpp"

#define FIXTURE_PATH "test/fixtures/"

// ファイル読み込み
//   引数
//      filepath: ファイル名 (tests/fixtures/ からの相対パス)
//      file_size: ファイルサイズを格納する変数へのポインタ
//   戻り値: ファイル内容を格納したバッファ (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char* testutil_read_fixture_to_buffer(const char* filepath, size_t* file_size) {
    // FIXTURE_PATH を付加したパスを生成
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", FIXTURE_PATH, filepath);

    return testutil_read_file_to_buffer(fullpath, file_size);
}

// ファイル読み込み (行ごとに分割)
//   引数
//      filepath: ファイル名 (tests/fixtures/ からの相対パス)
//      line_count: 行数を格納する変数へのポインタ
//   戻り値: 行ごとの文字列配列 (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char** testutil_read_fixture_to_lines(const char* filepath, size_t* line_count) {
    // FIXTURE_PATH を付加したパスを生成
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", FIXTURE_PATH, filepath);

    return testutil_read_file_to_lines(fullpath, line_count);
}

