#ifdef __APPLE__

#include "eventfd_shim.h"
#include <unistd.h>
#include <errno.h>

int eventfd(unsigned int initval, int flags) {
    int fds[2];
    (void)initval; // Suppress unused parameter warning

    if (pipe(fds) < 0) {
        return -1;
    }

    // Set both ends to non-blocking if requested
    if (flags & EFD_NONBLOCK) {
        if (fcntl(fds[0], F_SETFL, O_NONBLOCK) < 0 ||
            fcntl(fds[1], F_SETFL, O_NONBLOCK) < 0) {
            close(fds[0]);
            close(fds[1]);
            return -1;
        }
    }

    // Set close-on-exec if requested
    if (flags & EFD_CLOEXEC) {
        if (fcntl(fds[0], F_SETFD, FD_CLOEXEC) < 0 ||
            fcntl(fds[1], F_SETFD, FD_CLOEXEC) < 0) {
            close(fds[0]);
            close(fds[1]);
            return -1;
        }
    }

    // Write initial value if provided
    if (initval != 0) {
        uint64_t val = initval;
        if (write(fds[1], &val, sizeof(val)) != sizeof(val)) {
            close(fds[0]);
            close(fds[1]);
            return -1;
        }
    }

    // Close write end, we only need read end for signaling
    close(fds[1]);
    return fds[0];
}

#endif /* __APPLE__ */
