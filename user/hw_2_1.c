#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/sleeplock_nandler.h"
#include "user/user.h"

#define STDERR 2
#define min(a, b) ((a) < (b) ? (a) : (b))

int pid;

int safe_handle_sleeplock(int operation, int lock_id) {
    static char *err2msg[ERR_COUNT + 2] = {"", "invalid lock id\n", "invalid sleeplock state\n",
                                           "not enough sleeplocks\n", "wrong request type\n", "unexpected error\n"};

    int error_code = handle_sleeplock(operation, lock_id);
    if (error_code >= 0) return error_code;

    error_code = min(-error_code, ERR_COUNT + 1);

    fprintf(STDERR, err2msg[error_code]);
    if (pid && (operation == ACQUIRE_SLEEPLOCK || operation == RELEASE_SLEEPLOCK)) {
        handle_sleeplock(REMOVE_SLEEPLOCK, lock_id);
    }
    exit(1);
}

// Maybe it's better to use multiple "write()" and check the return value.
// But in this task it looks like overkill.
void tell_pids_char(char *c, int lock_id) {
    safe_handle_sleeplock(ACQUIRE_SLEEPLOCK, lock_id);
    printf("%d: received %c\n", getpid(), *c);
    safe_handle_sleeplock(RELEASE_SLEEPLOCK, lock_id);
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

    int lock_id = safe_handle_sleeplock(GET_SLEEPLOCK, 0);

    pid = fork();
    if (pid == -1) {
        fprintf(STDERR, "fork error\n");
        handle_sleeplock(REMOVE_SLEEPLOCK, lock_id);
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
        safe_handle_sleeplock(REMOVE_SLEEPLOCK, lock_id);
    }
    exit(0);
}
