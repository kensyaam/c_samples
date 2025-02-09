#include "common.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

// mkfifoのwrapper
int createFifo(fifo_t *fifo) {
    if (mkfifo(fifo->path, 0666) != 0) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! mkfifo for %s. %s\n", getpid(), fifo->path, strerror(errno));
        return 1;
    } else {
        printf("[%d] mkfifo for %s\n", getpid(), fifo->path);
        printf("foo bar\n");
        fifo->created = 1;
    }

    return 0;
}

// pollのwrapper
int pollFifo(fifo_t *fifo, short events, int timeoutMs) {
    struct pollfd pfd = {fifo->fd, events, 0};
    int ret;

    printf("[%d] poll for %s: ... \n", getpid(), fifo->path);
    fflush(stdout);
    ret = poll(&pfd, 1, timeoutMs);
    if (ret == -1) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! poll for %s. %s\n", getpid(), fifo->path, strerror(errno));
        return 1;
    } else if (ret == 0) {
        // タイムアウト
        fprintf(stderr, "[%d] TIMEOUT! poll for %s.\n", getpid(), fifo->path);
        return 1;
    } else {
        printf("[%d] done! revents=0x%x\n", getpid(), pfd.revents);
    }

    return 0;
}

// open, fdopenのwrapper
int openFifo(fifo_t *fifo, int flags, char *mode) {
    printf("[%d] open for %s: ... \n", getpid(), fifo->path);
    fflush(stdout);
    fifo->fd = open(fifo->path, flags);
    if (fifo->fd == -1) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! %d : %s\n", getpid(), errno, strerror(errno));
        return 1;
    } else {
        printf("[%d] done! fd=%d\n", getpid(), fifo->fd);
    }

    printf("[%d] fdopen for %s: ... \n", getpid(), fifo->path);
    fflush(stdout);
    fifo->fp = fdopen(fifo->fd, mode);
    if (fifo->fp == NULL) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! %s.\n", getpid(), fifo->path);
        return 1;
    } else {
        printf("[%d] done! fp=%p\n", getpid(), fifo->fp);
    }

    return 0;
}

// fclose or closeのwrapper
int closeFifo(fifo_t *fifo) {
    int ret = 0;

    if (fifo->fp != NULL) {
        ret = fclose(fifo->fp);
    } else if (fifo->fd != -1) {
        ret = close(fifo->fd);
    } else {
        // close済み
        return 0;
    }

    if (ret != 0) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! fclose (or close) for %s. %s\n", getpid(), fifo->path, strerror(errno));
        return 1;
    } else {
        printf("[%d] fclose (or close) for %s\n", getpid(), fifo->path);
        fifo->fd = -1;
        fifo->fp = NULL;
    }

    return 0;
}

// unlinkのwrapper
int deleteFifo(fifo_t *fifo) {
    closeFifo(fifo);

    if (fifo->created == 0) {
        return 0;
    }

    if (unlink(fifo->path) != 0) {
        // 失敗時
        fprintf(stderr, "[%d] FAILED! unlink for %s. %s\n", getpid(), fifo->path, strerror(errno));
        return 1;
    } else {
        printf("[%d] unlink for %s\n", getpid(), fifo->path);
        fifo->created = 0;
    }

    return 0;
}
