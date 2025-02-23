#ifndef EVENTFD_SHIM_H
#define EVENTFD_SHIM_H

#ifdef __APPLE__

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

/* eventfd flags */
#define EFD_NONBLOCK O_NONBLOCK
#define EFD_CLOEXEC  O_CLOEXEC
#define EFD_SEMAPHORE 1

typedef struct {
    int readfd;
    int writefd;
} eventfd_t;

static inline void eventfd_cleanup(eventfd_t *ctx) {
    if (ctx) {
        if (ctx->readfd >= 0) close(ctx->readfd);
        if (ctx->writefd >= 0) close(ctx->writefd);
    }
}

static inline int eventfd_signal(eventfd_t *ctx) {
    uint64_t val = 1;
    ssize_t res = write(ctx->writefd, &val, sizeof(val));
    return (res == sizeof(val)) ? 0 : -1;
}

static inline int eventfd_read(eventfd_t *ctx) {
    uint64_t val;
    ssize_t res = read(ctx->readfd, &val, sizeof(val));
    return (res == sizeof(val)) ? 0 : -1;
}

static inline eventfd_t *eventfd_create(int flags) {
    eventfd_t *ctx = calloc(1, sizeof(eventfd_t));
    if (!ctx) return NULL;

    ctx->readfd = -1;
    ctx->writefd = -1;

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        free(ctx);
        return NULL;
    }

    ctx->readfd = pipefd[0];
    ctx->writefd = pipefd[1];

    // Set flags if requested
    if (flags & EFD_NONBLOCK) {
        fcntl(ctx->readfd, F_SETFL, O_NONBLOCK);
        fcntl(ctx->writefd, F_SETFL, O_NONBLOCK);
    }
    if (flags & EFD_CLOEXEC) {
        fcntl(ctx->readfd, F_SETFD, FD_CLOEXEC);
        fcntl(ctx->writefd, F_SETFD, FD_CLOEXEC);
    }

    return ctx;
}

#endif /* __APPLE__ */
#endif /* EVENTFD_SHIM_H */
