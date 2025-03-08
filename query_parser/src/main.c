#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse_query(const char *url_path, int *values) {
    // 初期化
    // values[0] = values[1] = values[2] = -1;

    // クエリ部分を探す
    const char *query = strchr(url_path, '?');
    if (query == NULL) {
        return; // クエリがない場合は終了
    }
    query++; // '?'の次の文字から開始

    // クエリを解析
    char *query_copy = strdup(query);
    char *token = strtok(query_copy, "&");
    while (token != NULL) {
        printf("token: %s\n", token);
        int key_len = strcspn(token, "=");
        char key[key_len + 1];
        strncpy(key, token, key_len);
        key[key_len] = '\0';
        char *value = strchr(token, '=') + 1;
        if (key != NULL && value != NULL) {
            if (strcmp(key, "AAA") == 0) {
                values[0] = atoi(value);
            } else if (strcmp(key, "BBB") == 0) {
                values[1] = atoi(value);
            } else if (strcmp(key, "CCC") == 0) {
                values[2] = atoi(value);
            }
        }
        token = strtok(NULL, "&");
    }
    free(query_copy);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }
    printf("URL: %s\n", argv[1]);

    int values[3] = { 0, 0, 1 };
    parse_query(argv[1], values);
    printf("AAA: %d\n", values[0]);
    printf("BBB: %d\n", values[1]);
    printf("CCC: %d\n", values[2]);

    return 0;
}
