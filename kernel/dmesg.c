#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "dmesg.h"

#include <stdarg.h>

#define BUFF_SIZE_IN_PAGES 1
#define BUFF_SIZE (BUFF_SIZE_IN_PAGES * PGSIZE)

// Amount of characters counting spaces that record the number of ticks
#define TICKS_RECORD_LENGTH 7

static char buffer[BUFF_SIZE];
static int current_buffer_position = 0;
static int buffer_overlaps = 0;

static struct spinlock lock;
static struct spinlock settings_lock;

static char log_settings[COUNT];

void dmesginit() {
    initlock(&lock, "dmesg lock");

    log_settings[INTERRUPT] = 1;
    log_settings[SWITCH] = 1;
    log_settings[SYSCALL] = 1;
}

void increase_buffer_pos() {
    current_buffer_position++;
    if (current_buffer_position == BUFF_SIZE) {
        buffer_overlaps = 1;
        current_buffer_position = 0;
    }
}

static void append_char(char c) {
    buffer[current_buffer_position] = c;
    increase_buffer_pos();
}

static void append_str(const char *str) {
    while (*str != 0) {
        buffer[current_buffer_position] = *str;
        increase_buffer_pos();
        str++;
    }
}

static char *digit_symbols = "0123456789abcdef";

static void append_int_with_alignment(int n) {
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

#define MAX_BUFFER_SIZE 17

static void append_int(int n, int base) {
    char buffer[MAX_BUFFER_SIZE + 1];
    buffer[MAX_BUFFER_SIZE] = 0;
    char *buffer_head = buffer + MAX_BUFFER_SIZE;

    if (n == 0) {
        append_char('0');
        return;
    }

    if (n < 0) {
        append_char('-');
        n = -(long long) n;
    }

    while (n > 0) {
        *(--buffer_head) = digit_symbols[n % base];
        n /= base;
    }

    append_str(buffer_head);
}

static void append_ptr(uint64 x) {
    append_str("0x");
    for (int i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        append_char(digit_symbols[x >> (sizeof(uint64) * 8 - 4)]);
}

static void append_ticks_info() {
    int ticks_ = ticks;
    append_int_with_alignment(ticks_);
    append_str(" :: ");
}

char check_log_setting(enum settings setting) {
    acquire(&settings_lock);
    char result = log_settings[setting];
    release(&settings_lock);
    return result;
}

void pr_msg(const char *fmt, ...) {
    acquire(&lock);
    append_ticks_info();

    int i, c;
    char *s;

    if (fmt == 0)
        panic("null fmt");

    va_list ap;
    va_start(ap, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            append_char(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c) {
            case 'd':
                append_int(va_arg(ap,
                int), 10);
                break;
            case 'x':
                append_int(va_arg(ap,
                int), 16);
                break;
            case 'p':
                append_ptr(va_arg(ap, uint64));
                break;
            case 's':
                if ((s = va_arg(ap, char*)) == 0)
                s = "(null)";
                append_str(s);
                break;
            case '%':
                append_char('%');
                break;
            default:
                // Append unknown % sequence to draw attention.
                append_char('%');
                append_char(c);
                break;
        }
    }
    va_end(ap);
    append_char('\n');
    release(&lock);
}

uint64 sys_dmesg() {
    acquire(&lock);
    uint64 address;
    argaddr(0, &address);
    if (!buffer_overlaps) {
        if (copyout(myproc()->pagetable, address, buffer, sizeof(char) * current_buffer_position) < 0) {
            release(&lock);
            return -1;
        }
        release(&lock);
        return current_buffer_position;
    } else {
        if (copyout(myproc()->pagetable, address, buffer + current_buffer_position,
                    sizeof(char) * (BUFF_SIZE - current_buffer_position)) < 0) {
            release(&lock);
            return -1;
        }
        if (copyout(myproc()->pagetable, address + sizeof(char) * (BUFF_SIZE - current_buffer_position), buffer,
                    sizeof(char) * current_buffer_position) < 0) {
            release(&lock);
            return -1;
        }
        release(&lock);
        return BUFF_SIZE;
    }
}

uint64 sys_set_log_settings() {
    int setting;
    argint(0, &setting);
    if (setting >= COUNT || setting < 0)
        return -1;
    acquire(&settings_lock);
    log_settings[setting] ^= 1;
    release(&settings_lock);

    return 0;
}
