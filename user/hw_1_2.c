#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STDERR 2

// Maybe it's better to use multiple "write()" and check the return value.
// But in this task it looks like overkill.
void tell_pids_char(char *c) {
  printf("%d: received %c\n", getpid(), *c);
}

void safe_write_char(int bf, char *buffer) {
  if (write(bf, buffer, 1) != 1) {
    fprintf(STDERR, "write error\n");
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

  pipe(parent_2_child_pipe);
  pipe(child_2_parent_pipe);
  if (fork() == 0) {
    char *current_char = malloc(1);
    close(parent_2_child_pipe[1]);
    close(child_2_parent_pipe[0]);

    while (read(parent_2_child_pipe[0], current_char, 1) == 1) {
      tell_pids_char(current_char);

      safe_write_char(child_2_parent_pipe[1], current_char);
    }

    free(current_char);

    close(parent_2_child_pipe[0]);
    close(child_2_parent_pipe[1]);
  } else {
    close(parent_2_child_pipe[0]);
    close(child_2_parent_pipe[1]);

    char *current_char = argv[1];

    while (*current_char != 0) {
      safe_write_char(parent_2_child_pipe[1], current_char++);
    }

    close(parent_2_child_pipe[1]);

    while(read(child_2_parent_pipe[0], current_char, 1) == 1) {
      tell_pids_char(current_char++);
    }

    close(child_2_parent_pipe[0]);
  }
  exit(0);
}