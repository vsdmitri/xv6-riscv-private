#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STDERR 2

#define ACQUIRE 0
#define RELEASE 1
#define GET     2
#define REMOVE  3

void handle_sleeplock_handler_error(int error_code) {
    if (error_code >= 0) return;

    if (error_code == -1) {
        fprintf(STDERR, "invalid lock id\n");
        exit(1);
    }

    if (error_code == -2) {
        fprintf(STDERR, "invalid sleeplock state\n");
        exit(1);
    }

    if (error_code == -3) {
        fprintf(STDERR, "not enough sleeplocks\n");
        exit(1);
    }

    if (error_code == -4) {
        fprintf(STDERR, "wrong request type\n");
        exit(1);
    }

    fprintf(STDERR, "unexpected error in sleeplock handler\n");
    exit(1);
}

// Maybe it's better to use multiple "write()" and check the return value.
// But in this task it looks like overkill.
void tell_pids_char(char *c, int lock_id) {
    handle_sleeplock_handler_error(handle_sleeplock(ACQUIRE, lock_id));
    printf("%d: received %c\n", getpid(), *c);
    handle_sleeplock_handler_error(handle_sleeplock(RELEASE, lock_id));
}

void safe_write_char(int bf, char *buffer) {
    if (write(bf, buffer, 1) != 1) {
        fprintf(STDERR, "write error\n");
        exit(1);
    }
}

void safe_pipe_creation(int *p) {
    if (pipe(p) == -1) {
        fprintf(STDERR, "pipe creation error\n");
        exit(1);
    }
}

int main(int argc, char **argv) {
    int parent_2_child_pipe[2];
    int child_2_parent_pipe[2];

    if (argc < 2) {
        fprintf(STDERR, "missing argument\n");
        exit(1);
    }

    safe_pipe_creation(parent_2_child_pipe);
    safe_pipe_creation(child_2_parent_pipe);

    int lock_id = handle_sleeplock(GET, 0);
    handle_sleeplock_handler_error(lock_id);

    int pid = fork();
    if (pid == -1) {
        fprintf(STDERR, "fork error\n");
        exit(1);
    }

    if (pid == 0) {
        char current_char;
        close(parent_2_child_pipe[1]);
        close(child_2_parent_pipe[0]);

        while (read(parent_2_child_pipe[0], &current_char, 1) == 1) {
            tell_pids_char(&current_char, lock_id);

            safe_write_char(child_2_parent_pipe[1], &current_char);
        }

        close(parent_2_child_pipe[0]);
        close(child_2_parent_pipe[1]);
    } else {
        close(parent_2_child_pipe[0]);
        close(child_2_parent_pipe[1]);

        char *current_char = argv[1];

        while (*current_char != 0) {
            safe_write_char(parent_2_child_pipe[1], current_char++);
        }

        char new_current_char;

        close(parent_2_child_pipe[1]);

        while (read(child_2_parent_pipe[0], &new_current_char, 1) == 1) {
            tell_pids_char(&new_current_char, lock_id);
        }

        close(child_2_parent_pipe[0]);
        wait(0);
        handle_sleeplock(REMOVE, lock_id);
    }
    exit(0);
}
