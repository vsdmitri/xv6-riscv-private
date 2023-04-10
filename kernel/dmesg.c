#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define BUFF_SIZE_IN_PAGES 1
#define BUFF_SIZE (BUFF_SIZE_IN_PAGES * PGSIZE)

// Amount of characters counting spaces that record the number of ticks
#define TICKS_RECORD_LENGTH 11

static char buffer[BUFF_SIZE];
static int current_buffer_size = 0;

static struct spinlock lock;

extern int consolewrite(int, uint64, int);

void dmesginit() {
    initlock(&lock, "dmesg lock");
}

static void append_str(const char *str) {
    while (current_buffer_size < BUFF_SIZE && *str != 0) {
        buffer[current_buffer_size++] = *str;
        str++;
    }
}

static void append_int(int n) {
    char digits[TICKS_RECORD_LENGTH + 1];
    digits[TICKS_RECORD_LENGTH] = 0;

    char *first_digit = digits + TICKS_RECORD_LENGTH;
    if (n == 0) {
        *(--first_digit) = '0';
    }

    while (n > 0) {
        *(--first_digit) = '0' + (n % 10);
        n /= 10;
    }

    while (first_digit != digits) *(--first_digit) = ' ';
    append_str(digits);
}

static void append_ticks_info() {
    append_str("Ticks =");
    acquire(&tickslock);
    int ticks_ = ticks;
    release(&tickslock);
    append_int(ticks_);
    append_str(" :: ");
}

void pr_msg(const char *str) {
    acquire(&lock);
    append_ticks_info();
    append_str(str);
    append_str("\n");
    release(&lock);
}

void dmesg() {
    consolewrite(0, (uint64)buffer, current_buffer_size);
}
