#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STDIN 0
#define STDERR 2

int main() {
  int p[2];
  char *argv[2];
  argv[0] = "wc";
  argv[1] = 0;

  pipe(p);
  if (fork() == 0) {
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);

    if (exec("wc", argv) != 0) {
      fprintf(STDERR, "wc returned an error\n");
      exit(1);
    }
  } else {
    close(p[0]);

    const int buffer_max_size = 256;
    char buffer[buffer_max_size];
    int bytes_read = read(STDIN, buffer, sizeof buffer);

    if (bytes_read < 0) {
      fprintf(STDERR, "read error\n");
      exit(1);
    }

    if (bytes_read != write(p[1], buffer, bytes_read)) {
      fprintf(STDERR, "write in pipe error\n");
      exit(1);
    }

    close(p[1]);
    wait(0);
  }
  exit(0);
}
