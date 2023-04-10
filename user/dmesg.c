#include "kernel/types.h"
#include "user/user.h"

#define MAX_BUFF_SIZE 3333
int main() {
    char buffer[MAX_BUFF_SIZE];
    int result = dmesg(buffer);
    if (result < 0) {
        printf("Dmesg erorr!\n");
        exit(1);
    }

    for(int i = 0; i < result && i < MAX_BUFF_SIZE; i++) {
        write(1, buffer + i,  1);
    }
    exit(0);
}
