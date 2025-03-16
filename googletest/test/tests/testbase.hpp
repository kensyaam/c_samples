#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdbool.h>

#include "gtest/gtest.h"
#include "fff.h"

extern "C" {

    DEFINE_FFF_GLOBALS;

    // Mock definitions
    FAKE_VALUE_FUNC(void*, malloc_wrapped, size_t);
    FAKE_VALUE_FUNC(void*, realloc_wrapped, void *, size_t);
    FAKE_VOID_FUNC(free_wrapped, void*);
    
}

class TBase : public ::testing::Test {
protected:
    virtual void SetUp(){
        RESET_FAKE(malloc_wrapped);
        RESET_FAKE(realloc_wrapped);
        RESET_FAKE(free_wrapped);
        //  デフォルトのmalloc, realloc, freeを使う
        malloc_wrapped_fake.custom_fake = malloc;
        realloc_wrapped_fake.custom_fake = realloc;
        free_wrapped_fake.custom_fake = free;

        FFF_RESET_HISTORY();
    }

    virtual void TearDown(){
    }

    // ---------------------------------------------------
    // コマンド実行
    // ---------------------------------------------------

    // コマンド実行結果の読み込み（標準出力のみ）
    //   引数
    //      cmd: 実行するコマンド
    //      stdout_size: 標準出力のサイズを格納する変数へのポインタ (不要ならNULL)
    //   戻り値: 標準出力の内容を格納したバッファ (エラー時は NULL)
    //   注意: 戻り値のメモリは呼び出し元で解放する必要がある
    char* testutil_exec_cmd_and_read_output(const char* cmd, size_t* stdout_size = NULL) {
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
        if (stdout_size != NULL) {
            *stdout_size = total_read;
        }
        buffer[total_read] = '\0'; // 終端文字を追加
        return buffer;    
    }

    // コマンド実行 (system 関数のラッパー)
    void testutil_exec_cmd(const char *cmd) {
        system(cmd);
    }

    // コマンド実行 - 別プロセスで実行し、標準出力をファイルに保存
    void testutil_exec_cmd_async(const char *cmd, const char *output_path = NULL) {
        // forkして子プロセスでcmdで指定されたコマンドを実行する。
        // コマンドの標準出力をoutput_pathで指定されたファイルに保存する。
        pid_t pid = fork();
        if (pid == 0) {
            // 子プロセス
            char *output = testutil_exec_cmd_and_read_output(cmd, NULL);
            if (output != NULL && output_path != NULL) {
                FILE *file = fopen(output_path, "w");
                if (file != NULL) {
                    fprintf(file, "%s", output);
                    fclose(file);
                }
                free(output);
            }
            exit(0);
        } else {
            // 親プロセス
        }
    }

    // ---------------------------------------------------
    // ファイル読み込み
    // ---------------------------------------------------

    // ファイル読み込み
    //   引数
    //      filepath: ファイル名 (作業ディレクトリからの相対パス or 絶対パス)
    //      size: ファイルサイズを格納する変数へのポインタ (不要ならNULL)
    //   戻り値: ファイル内容を格納したバッファ (エラー時は NULL)
    //   注意: 戻り値のメモリは呼び出し元で解放する必要がある
    char* testutil_read_file_to_buffer(const char* filepath, size_t* size) {
        const char *fullpath = filepath;
        size_t file_size = 0;

        FILE* file = fopen(fullpath, "rb"); // バイナリモードでオープン
        if (file == NULL) {
            printf("testutil - open failed: fullpath=%s\n", fullpath);
            return NULL; // ファイルオープン失敗
        }

        // ファイルサイズの取得
        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // バッファの確保
        char* buffer = (char*)malloc(file_size + 1); // 終端文字用に+1
        if (buffer == NULL) {
            fclose(file);
            printf("testutil - malloc failed: file_size=%d\n", file_size);
            return NULL; // メモリ確保失敗
        }

        // ファイル内容の読み込み
        size_t read_size = fread(buffer, 1, file_size, file);
        fclose(file);

        if (read_size != file_size) {
            free(buffer);
            printf("testutil - ファイルサイズ不整合: file_size=%d, read_size=%d\n", file_size, read_size);
            return NULL; // 読み込みエラー
        }

        buffer[file_size] = '\0'; // 終端文字を追加
        if (size != NULL) {
            *size = file_size;
        }
        return buffer;
    }

    // ファイル読み込み (行ごとに分割)
    //   引数
    //      filepath: ファイル名 (作業ディレクトリからの相対パス or 絶対パス)
    //      line_count: 行数を格納する変数へのポインタ
    //   戻り値: 行ごとの文字列配列 (エラー時は NULL)
    //   注意: 戻り値のメモリは呼び出し元で解放する必要がある
    char** testutil_read_file_to_lines(const char* filepath, size_t* line_count) {
        const char *fullpath = filepath;

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

    // ---------------------------------------------------
    // ファイル読み込み（test/fixtures/ から）
    // ---------------------------------------------------
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

    // ---------------------------------------------------
    // curl
    // ---------------------------------------------------
    // レスポンスデータを格納する構造体
    typedef struct TestUtilResponseData {
        char* data;
        size_t size;
        int status_code;
    } TestUtilResponseData;

    // レスポンスデータ書き込みコールバック関数
    static size_t testutil_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        size_t real_size = size * nmemb;
        TestUtilResponseData* response = (TestUtilResponseData*)userdata;
        response->data = (char*)realloc(response->data, response->size + real_size + 1);
        if (response->data == NULL) {
            return 0;
        }
        memcpy(&(response->data[response->size]), ptr, real_size);
        response->size += real_size;
        response->data[response->size] = '\0';
        return real_size;
    }

    // HTTPリクエスト関数
    TestUtilResponseData* testutil_http_request(const char* method, const char* url, 
            const char* headers[], size_t header_count, 
            const char* data = NULL, const char* http_version = "1.1") {
        curl_global_init(CURL_GLOBAL_ALL);
        CURL* curl = curl_easy_init();
        if (curl == NULL) {
            return NULL;
        }
    
        TestUtilResponseData *response = (TestUtilResponseData *)malloc(sizeof(TestUtilResponseData));
        response->data = NULL;
        response->size = 0;
        response->status_code = 0;
    
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, testutil_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
    
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // SSL証明書検証を無効化
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // デバッグ情報出力

        // HTTPメソッド設定
        if (strcmp(method, "POST") == 0) {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if (strcmp(method, "PUT") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        } else if (strcmp(method, "DELETE") == 0) {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        } else {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
        }
    
        // HTTPバージョン設定
        if (strcmp(http_version, "1.0") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
        } else if (strcmp(http_version, "1.1") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        } else if (strcmp(http_version, "2.0") == 0) {
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        }
    
        // ヘッダー設定
        struct curl_slist* header_list = NULL;
        bool contains_content_length = false;
        for (size_t i = 0; i < header_count; i++) {
            header_list = curl_slist_append(header_list, headers[i]);
            if (strstr(headers[i], "Content-Length:") != NULL) {
                contains_content_length = true;
            }
        }
        if (header_list != NULL) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        }
    
        if (data != NULL && !contains_content_length) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data)); // Content-Length自動設定
        }
    
        CURLcode res = curl_easy_perform(curl);
    
        if (res != CURLE_OK) {
            free(response->data);
            free(response);
            response = NULL;
        }
    
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->status_code); // ステータスコード取得

    cleanup:
        curl_easy_cleanup(curl);
        curl_slist_free_all(header_list);
        curl_global_cleanup();

        return response;
    }

    // レスポンスデータを解放する関数
    void testutil_free_response_data(TestUtilResponseData* response) {
        if (response != NULL) {
            free(response->data);
            free(response);
        }
    }

    // HTTPリクエスト関数 - 別プロセスで実行し、結果をファイルに保存(1行目: ステータスコード, 2行目以降: レスポンスデータ)
    void testutil_http_request_async(const char *output_path, const char* method, const char* url, 
        const char* headers[], size_t header_count, 
        const char* data = NULL, const char* http_version = "1.1") {
        // forkして子プロセスでcmdで指定されたコマンドを実行する。
        // コマンドの標準出力をoutput_pathで指定されたファイルに保存する。
        pid_t pid = fork();
        if (pid == 0) {
            // 子プロセス
            TestUtilResponseData *resp = testutil_http_request(method, url, headers, header_count, data, http_version);
            if (resp != NULL && output_path != NULL) {
                FILE *file = fopen(output_path, "w");
                if (file != NULL) {
                    fprintf(file, "%d\n", resp->status_code);
                    fprintf(file, "%s", resp->data);
                    fclose(file);
                }
                free(resp);
            }
            exit(0);
        } else {
            // 親プロセス
        }
    }

};

