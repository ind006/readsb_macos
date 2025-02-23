#ifndef EPOLL_EVENT_COMPAT_H
#define EPOLL_EVENT_COMPAT_H

#ifdef __APPLE__

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

// Epoll compatibility for main event loop
typedef struct {
    int kq;                  // kqueue file descriptor
    struct kevent *events;   // events array
    int maxevents;          // size of events array
} epoll_context;

static inline epoll_context* epoll_create_context(int maxevents, int *exitfd) {
    epoll_context *ctx = calloc(1, sizeof(epoll_context));
    if (!ctx) return NULL;

    ctx->kq = kqueue();
    if (ctx->kq < 0) {
        free(ctx);
        return NULL;
    }

    ctx->events = calloc(maxevents, sizeof(struct kevent));
    if (!ctx->events) {
        close(ctx->kq);
        free(ctx);
        return NULL;
    }

    ctx->maxevents = maxevents;

    // Add exitfd to kqueue
    struct kevent ev;
    EV_SET(&ev, *exitfd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(ctx->kq, &ev, 1, NULL, 0, NULL) < 0) {
        close(ctx->kq);
        free(ctx->events);
        free(ctx);
        return NULL;
    }

    return ctx;
}

static inline int epoll_wait_events(epoll_context *ctx, int timeout_ms) {
    struct timespec ts;
    struct timespec *timeout = NULL;
    
    if (timeout_ms >= 0) {
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (timeout_ms % 1000) * 1000000;
        timeout = &ts;
    }

    return kevent(ctx->kq, NULL, 0, ctx->events, ctx->maxevents, timeout);
}

static inline void epoll_destroy_context(epoll_context *ctx) {
    if (ctx) {
        if (ctx->kq >= 0)
            close(ctx->kq);
        free(ctx->events);
        free(ctx);
    }
}

// Check if event is ready for reading
static inline int epoll_event_is_ready(epoll_context *ctx, int index) {
    return (ctx->events[index].filter == EVFILT_READ);
}

#endif /* __APPLE__ */
#endif /* EPOLL_EVENT_COMPAT_H */
