#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define BUFF_SIZE_IN_PAGES 1
#define BUFF_SIZE (BUFF_SIZE_IN_PAGES * PGSIZE)

static char buffer[BUFF_SIZE];

int main() {
    int result = dmesg(buffer);
    if (result < 0) {
        printf("Dmesg erorr!\n");
        exit(1);
    }

    for(int i = 0; i < result && i < BUFF_SIZE; i++) {
        write(1, buffer + i,  1);
    }
    exit(0);
}
