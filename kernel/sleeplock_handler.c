#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "defs.h"

#define ACQUIRE 0
#define RELEASE 1
#define GET     2
#define REMOVE  3

struct {
    struct spinlock lock;
    struct sleeplock sleeplocks[NSLEEPLOCK];
    char sleeplock_used[NSLEEPLOCK];
} sleeplock_handler;

void sleeplock_handler_init() {
    initlock(&sleeplock_handler.lock, "sleeplock_handler");
    for (int i = 0; i < NSLEEPLOCK; i++) {
        initsleeplock(&sleeplock_handler.sleeplocks[i], "available_sleeplock");
        sleeplock_handler.sleeplock_used[i] = 0;
    }
}

int check_lock_id(int lock_id) {
    if (lock_id < 0 || lock_id >= NSLEEPLOCK)
        return -1; // invalid lock_id

    if (!sleeplock_handler.sleeplock_used[lock_id])
        return -2; // not busy

    return 0;
}

uint64 handle_sleeplock(int request_type, int lock_id) {
    int error = 0;

    if (request_type == ACQUIRE) {
        error = check_lock_id(lock_id);
        if (error < 0) return error;
        acquiresleep(&sleeplock_handler.sleeplocks[lock_id]);
        return 0;
    }

    if (request_type == RELEASE) {
        error = check_lock_id(lock_id);
        if (error < 0) return error;
        releasesleep(&sleeplock_handler.sleeplocks[lock_id]);
        return 0;
    }

    if (request_type == GET) {
        acquire(&sleeplock_handler.lock);
        int free = 0;
        for (; free < NSLEEPLOCK; free++) {
            if (!sleeplock_handler.sleeplock_used[free]) {
                sleeplock_handler.sleeplock_used[free] = 1;
                release(&sleeplock_handler.lock);
                return free;
            }
        }
        release(&sleeplock_handler.lock);

        return -3; // to many locks
    }

    if (request_type == REMOVE) {
        acquire(&sleeplock_handler.lock);
        error = check_lock_id(lock_id);
        if (error < 0) {
            release(&sleeplock_handler.lock);
            return error;
        }
        sleeplock_handler.sleeplock_used[lock_id] = 0;
        release(&sleeplock_handler.lock);
        return 0;
    }

    return -4; // wrong request type
}
