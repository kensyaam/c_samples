// 受信側の例
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int main(int argc, char *argv[]) {
    fifo_t req  = {FIFO_REQ,  0, -1, NULL};
    fifo_t resp = {FIFO_RESP, 0, -1, NULL};
    char buffer[1024];
    int ret;
    int step_by_step = 0;

    // getoptを使って引数を解析する
    // 引数に -s がある場合はステップ実行
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1) {
        switch (opt) {
            case 's':
                step_by_step = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-s]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (step_by_step) {
        printf("[%d] Press any key to open pipe for request\n", getpid());
        (void) getc (stdin);
    }

    // 受信用パイプを開く
    if (openFifo(&req, O_RDONLY | O_NONBLOCK, "r") != 0) {
        return EXIT_FAILURE;
    }

    if (step_by_step) {
        printf("[%d] Press any key to receive data\n", getpid());
        (void) getc (stdin);
    }

    if (pollFifo(&req, POLLIN, 60*1000) != 0) {
        closeFifo(&req);
        return EXIT_FAILURE;
    }

    // データを受信
    while(fgets(buffer, sizeof(buffer), req.fp) != NULL) {
        printf("[%d] Received data: %s", getpid(), buffer);
    }

    if (step_by_step) {
        printf("[%d] Press any key to close pipe for request\n", getpid());
        (void) getc (stdin);
    }

    // 受信用パイプを閉じる
    if (closeFifo(&req) != 0) {
        return EXIT_FAILURE;
    }

    printf("[%d] ----------------\n", getpid());

    if (step_by_step) {
        printf("[%d] Press any key to open pipe for response\n", getpid());
        (void) getc (stdin);
    }

    // 送信用パイプを開く
    if (openFifo(&resp, O_WRONLY, "w") != 0) {
        return EXIT_FAILURE;
    }

    if (step_by_step) {
        printf("[%d] Press any key to send data\n", getpid());
        (void) getc (stdin);
    }

    // データを送信
    fprintf(resp.fp, "%s\n", "Goodnight, ");
    fprintf(resp.fp, "%s", "moon!\n");

    if (step_by_step) {
        printf("[%d] Press any key to close pipe for response\n", getpid());
        (void) getc (stdin);
    }

    // 送信用パイプを閉じる
    closeFifo(&resp);

    return EXIT_SUCCESS;
}
