#include <stdio.h>
#include <stdlib.h>

// コマンド実行結果の読み込み（標準出力のみ）
//   引数
//      cmd: 実行するコマンド
//      stdout_size: 標準出力のサイズを格納する変数へのポインタ
//   戻り値: 標準出力の内容を格納したバッファ (エラー時は NULL)
//   注意: 戻り値のメモリは呼び出し元で解放する必要がある
char* exec_cmd_and_read_output(const char* cmd, size_t* stdout_size) {
    FILE* fp = popen(cmd, "r");
    if (fp == NULL) {
        return NULL;
    }

    // ファイルサイズの取得
    fseek(fp, 0, SEEK_END);
    *stdout_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // バッファの確保
    char* buffer = (char*)malloc(*stdout_size + 1); // 終端文字用に+1
    if (buffer == NULL) {
        pclose(fp);
        return NULL; // メモリ確保失敗
    }

    // ファイル内容の読み込み
    size_t read_size = fread(buffer, 1, *stdout_size, fp);
    pclose(fp);

    if (read_size != *stdout_size) {
        free(buffer);
        return NULL; // 読み込みエラー
    }

    buffer[*stdout_size] = '\0'; // 終端文字を追加
    return buffer;
}

// コマンド実行 (system 関数のラッパー)
void exec_cmd(char *cmd) {
    system(cmd);
}
