#ifndef __INCLUDE_COMMON_H__
#define __INCLUDE_COMMON_H__

#include <stdio.h>
#include <poll.h>

#define FIFO_REQ "/tmp/myfifo_req"
#define FIFO_RESP "/tmp/myfifo_resp"

typedef struct {
    const char *path;
    int created;
    int fd;
    FILE *fp;
} fifo_t;

// mkfifoのwrapper
extern int createFifo(fifo_t *fifo);

// pollのwrapper
extern int pollFifo(fifo_t *fifo, short events, int timeoutMs);

// open, fdopenのwrapper
extern int openFifo(fifo_t *fifo, int flags, char *mode);

// fclose or closeのwrapper
extern int closeFifo(fifo_t *fifo);

// unlinkのwrapper
extern int deleteFifo(fifo_t *fifo);

#endif
