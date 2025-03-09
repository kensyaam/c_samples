extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

// コマンド実行結果の読み込み（標準出力のみ）
//   引数
//      cmd: 実行するコマンド
//      stdout_size: 標準出力のサイズを格納する変数へのポインタ
//   戻り値: 標準出力の内容を格納したバッファ (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char* testutil_exec_cmd_and_read_output(const char* cmd, size_t* stdout_size) {
    FILE* fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("testutil - popen failed: %s\n", cmd);
        return NULL;
    }

    // バッファの初期サイズ
    size_t buffer_size = 1024;
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        pclose(fp);
        printf("testutil - malloc failed\n");
        return NULL;
    }

    size_t total_read = 0;
    char temp[256];
    while (fgets(temp, sizeof(temp), fp) != NULL) {
        size_t len = strlen(temp);
        if (total_read + len >= buffer_size) {
            buffer_size *= 2;
            char* new_buffer = (char*)realloc(buffer, buffer_size);
            if (new_buffer == NULL) {
                free(buffer);
                pclose(fp);
                printf("testutil - realloc failed\n");
                return NULL;
            }
            buffer = new_buffer;
        }
        strcpy(buffer + total_read, temp);
        total_read += len;
    }

    pclose(fp);
    *stdout_size = total_read;
    buffer[total_read] = '\0'; // 終端文字を追加
    return buffer;
}

// コマンド実行 (system 関数のラッパー)
void testutil_exec_cmd(const char *cmd) {
    system(cmd);
}
