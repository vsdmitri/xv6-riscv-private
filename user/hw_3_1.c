#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    if (vmprint() != 0) {
        printf("No proc in pgaccess.\n");
        exit(1);
    }
    exit(0);
}
