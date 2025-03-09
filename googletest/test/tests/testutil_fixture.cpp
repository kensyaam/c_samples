extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#define FIXTURE_PATH "test/fixtures/"

// ファイル読み込み
//   引数
//      filepath: ファイル名 (tests/fixtures/ からの相対パス)
//      file_size: ファイルサイズを格納する変数へのポインタ
//   戻り値: ファイル内容を格納したバッファ (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char* testutil_read_file_to_buffer(const char* filepath, size_t* file_size) {
    // FIXTURE_PATH を付加したパスを生成
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", FIXTURE_PATH, filepath);

    FILE* file = fopen(fullpath, "rb"); // バイナリモードでオープン
    if (file == NULL) {
        printf("testutil - open failed: fullpath=%s\n", fullpath);
        return NULL; // ファイルオープン失敗
    }

    // ファイルサイズの取得
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // バッファの確保
    char* buffer = (char*)malloc(*file_size + 1); // 終端文字用に+1
    if (buffer == NULL) {
        fclose(file);
        printf("testutil - malloc failed: file_size=%d\n", file_size);
        return NULL; // メモリ確保失敗
    }

    // ファイル内容の読み込み
    size_t read_size = fread(buffer, 1, *file_size, file);
    fclose(file);

    if (read_size != *file_size) {
        free(buffer);
        printf("testutil - ファイルサイズ不整合: file_size=%d, read_size=%d\n", file_size, read_size);
        return NULL; // 読み込みエラー
    }

    buffer[*file_size] = '\0'; // 終端文字を追加
    return buffer;
}

// ファイル読み込み (行ごとに分割)
//   引数
//      filepath: ファイル名 (tests/fixtures/ からの相対パス)
//      line_count: 行数を格納する変数へのポインタ
//   戻り値: 行ごとの文字列配列 (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char** testutil_read_file_to_lines(const char* filepath, size_t* line_count) {
    // FIXTURE_PATH を付加したパスを生成
    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "%s%s", FIXTURE_PATH, filepath);

    FILE* file = fopen(fullpath, "r");
    if (file == NULL) {
        return NULL;
    }

    char** lines = NULL;
    char* line = NULL;
    size_t len = 0;
    *line_count = 0;

    while (getline(&line, &len, file) != -1) {
        // lineのコピーを作成
        char *line_copy = strdup(line);
        free(line);

        lines = (char**)realloc(lines, sizeof(char*) * (*line_count + 1));
        lines[*line_count] = line_copy;
        // printf("lines[%d]: %s: %x %x\n", *line_count, lines[*line_count], 
        //     &lines[*line_count], line_copy);
        // fflush(stdout);
        (*line_count)++;

        line = NULL;
        len = 0;
    }
    if (line != NULL) {
        free(line);
    }

    fclose(file);
    return lines;
}

// testutil_read_file_to_lines:（ファイル読み込み (行ごとに分割)） の戻り値の解放
//   引数
//      lines: 行ごとの文字列配列
void testutil_free_lines(char** lines, size_t line_count) {
    for (size_t i = 0; i < line_count; i++) {
        // printf("free(lines[%d]): 0x%x\n", i, lines[i]);
        free(lines[i]);
    }
    // printf("free(lines): 0x%x\n", lines);
    free(lines);
}

