// 送信側の例
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include "common.h"

void cleanup() {
    const char basedir[] = "/tmp";
    const char prefix[] = "myfifo_";

    // basedir にある prefix で始まる名前付きパイプを削除する
    DIR *dir;
    struct dirent *entry;
    char filepath[PATH_MAX];

    dir = opendir(basedir);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            snprintf(filepath, sizeof(filepath), "%s/%s", basedir, entry->d_name);
            if (unlink(filepath) != 0) {
                perror("unlink");
            } else {
                printf("Deleted FIFO: %s\n", filepath);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    fifo_t req  = {FIFO_REQ,  0, -1, NULL};
    fifo_t resp = {FIFO_RESP, 0, -1, NULL};
    char buffer[1024];
    int ret;
    int pid;

    int step_by_step = 0;
    int exec_receiver = 0;

    // getoptを使って引数を解析する
    // 引数に -s がある場合はステップ実行
    // 引数に -r がある場合は受信側を起動
    int opt;
    while ((opt = getopt(argc, argv, "sr")) != -1) {
        switch (opt) {
            case 's':
                step_by_step = 1;
                break;
            case 'r':
                exec_receiver = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-s] [-r]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    cleanup();

    printf("[%d] Press any key to start\n", getpid());
    (void) getc (stdin);

    // 名前付きパイプの作成
    if (createFifo(&req) != 0) {
        return EXIT_FAILURE;
    }
    if (createFifo(&resp) != 0) {
        deleteFifo(&req);
        return EXIT_FAILURE;
    }

    if (exec_receiver) {
        pid = fork();
        if (pid == 0) {
            // 子プロセスは受信側
            execl("./receiver", "receiver", NULL);
            perror("execl");
            return EXIT_FAILURE;
        } else if (pid == -1) {
            perror("fork");
            deleteFifo(&req);
            deleteFifo(&resp);
            return EXIT_FAILURE;
        }
        printf("[%d] forked %d\n", getpid(), pid);
    }

    if (step_by_step) {
        printf("[%d] Press any key to open pipe for request\n", getpid());
        (void) getc (stdin);
    }

    // 送信用パイプを開く
    for (int i = 0; i < 5; i++) {
        ret = openFifo(&req, O_WRONLY | O_NONBLOCK, "w");
        if (ret == 0) {
            break;
        } else {
            if (i < 4 && errno == ENXIO) {
                // パイプが開かれていない場合はリトライ
                printf("[%d] Retry open pipe for request after 1s\n", getpid());
                sleep(1);
            } else {
                deleteFifo(&req);
                deleteFifo(&resp);
                return EXIT_FAILURE;
            }
        }
    }
    // if (openFifo(&req, O_WRONLY | O_NONBLOCK , "w") != 0) {
    //     deleteFifo(&req);
    //     deleteFifo(&resp);
    //     return EXIT_FAILURE;
    // }

    // // 書き込みが停止 (block) しない状態になるのを待つ
    // if (pollFifo(&req, POLLOUT, 10*1000) != 0) {
    //     deleteFifo(&req);
    //     deleteFifo(&resp);
    //     return EXIT_FAILURE;
    // }

    if (step_by_step) {
        printf("[%d] Press any key to send data\n", getpid());
        (void) getc (stdin);
    }

    // 送信用データを送信
    fprintf(req.fp, "%s\n", "Hello, ");
    fprintf(req.fp, "\n");
    fprintf(req.fp, "%s\n", "World!");

    if (step_by_step) {
        printf("[%d] Press any key to close pipe for request\n", getpid());
        (void) getc (stdin);
    }

    // 送信用パイプを閉じる
    closeFifo(&req);
   
    printf("[%d] ----------------\n", getpid());

    if (step_by_step) {
        printf("[%d] Press any key to open pipe for response\n", getpid());
        (void) getc (stdin);
    }

    // 受信用パイプを開く
    if (openFifo(&resp, O_RDONLY | O_NONBLOCK, "r") !=0) {
        deleteFifo(&req);
        deleteFifo(&resp);
        return EXIT_FAILURE;
    }

    if (step_by_step) {
        printf("[%d] Press any key to receive data\n", getpid());
        (void) getc (stdin);
    }

    if (pollFifo(&resp, POLLIN, 60*1000) != 0) {
        deleteFifo(&req);
        deleteFifo(&resp);
        return EXIT_FAILURE;
    }

    // データを受信
    while(fgets(buffer, sizeof(buffer), resp.fp) != NULL) {
        printf("[%d] Received data: %s", getpid(), buffer);
    }

    if (step_by_step) {
        printf("[%d] Press any key to close pipe for response\n", getpid());
        (void) getc (stdin);
    }

    // 送信用パイプを閉じる
    closeFifo(&resp);

    // パイプを削除
    deleteFifo(&req);
    deleteFifo(&resp);

    return EXIT_SUCCESS;
}

