#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

static void print_invalid_usage() {
    printf("Invalid arguments!\n"
           "Usage:\n"
           "  --blocks <N> <fileName>\n"
           "or\n"
           "  --bytes <N> <fileName>\n");
}

#define BUF_SIZE 65536

static void create_file(int bytes, char *file_name) {
    static char was_memset = 0;
    static char buffer[BUF_SIZE];
    if (!was_memset) {
        memset(buffer, 0, BUF_SIZE);
        was_memset = 1;
    }

    int file = open(file_name, O_WRONLY | O_CREATE | O_TRUNC);
    if (file < 0) {
        printf("File open error.\n");
        exit(1);
    }

    for (int i = 0; i * BUF_SIZE < bytes; i++) {
        int size = min(BUF_SIZE, bytes - i * BUF_SIZE);
        if (write(file, buffer, size) != size) {
            printf("File filling error.\n");
            close(file);
            exit(1);
        }
    }

    close(file);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        print_invalid_usage();
        exit(1);
    }

    int factor = 0;
    if (strcmp(argv[1], "--blocks") == 0) {
        factor = BSIZE;
    } else if (strcmp(argv[1], "--bytes") == 0) {
        factor = 1;
    } else {
        print_invalid_usage();
        exit(1);
    }

    int n = atoi(argv[2]);
    if (n < 0) {
        print_invalid_usage();
        exit(1);
    }

    create_file(n * factor, argv[3]);
    exit(0);
}
