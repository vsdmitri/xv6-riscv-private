#include "kernel/types.h"
#include "kernel/dmesg.h"
#include "user/user.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Invalid arguments");
        exit(0);
    }

    if(strcmp(argv[1], "--interrupt") == 0 || strcmp(argv[1], "-i") == 0) {
        set_log_settings(INTERRUPT);
        exit(0);
    }

    if(strcmp(argv[1], "--switch") == 0 || strcmp(argv[1], "-s") == 0) {
        set_log_settings(SWITCH);
        exit(0);
    }

    if(strcmp(argv[1], "--syscall") == 0 || strcmp(argv[1], "-S") == 0) {
        set_log_settings(SYSCALL);
        exit(0);
    }

    printf("Invalid option. Use --interrupt, --switch or --syscall\n");
    exit(1);
}
