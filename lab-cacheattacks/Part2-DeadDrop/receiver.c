#include "util.h"
#include <sys/mman.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BUFF_SIZE (1<<21)
#define L2_WAYS 16
#ifndef STRIDE
#define STRIDE (1<<17)
#endif

#define SET_SPACING 32
#define BASE_SET 64

static inline uint64_t rdtscp(void) {
    uint32_t lo, hi;
    asm volatile("rdtscp" : "=a"(lo), "=d"(hi) :: "rcx");
    return ((uint64_t)hi << 32) | lo;
}

struct node {
    struct node *next;
    char pad[64 - sizeof(struct node *)];
};

void *buf;
struct node *sets[256];
uint64_t thresholds[256];

void shuffle(struct node **array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        struct node *temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void build_set(int logical_set_index) {
    char *base = (char *)buf;
    struct node *nodes[L2_WAYS];

    int physical_set_index = BASE_SET + (logical_set_index * SET_SPACING);

    for (int i = 0; i < L2_WAYS; i++) {
        nodes[i] = (struct node *)(base + physical_set_index * 64 + i * STRIDE);
    }

    shuffle(nodes, L2_WAYS);

    for (int i = 0; i < L2_WAYS - 1; i++) {
        nodes[i]->next = nodes[i+1];
    }
    nodes[L2_WAYS-1]->next = NULL;

    sets[logical_set_index] = nodes[0];
}

void prime_set(int set_index) {
    struct node *curr = sets[set_index];
    while (curr) curr = curr->next;
}

uint64_t probe_set(int set_index) {
    uint64_t start = rdtscp();
    struct node *curr = sets[set_index];
    while (curr) curr = curr->next;
    uint64_t end = rdtscp();
    return end - start;
}

void calibrate(uint64_t manual_threshold) {

    for (int i = 0; i <= 8; i++) {

        if (manual_threshold > 0) {
            thresholds[i] = manual_threshold;
        }

        else {

            uint64_t sum = 0;
            int samples = 1000;

            for (int k = 0; k < samples; k++) {
                prime_set(i);
                sum += probe_set(i);
            }

            uint64_t avg_hit = sum / samples;

            thresholds[i] = avg_hit * 1.5;
        }
    }
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    uint64_t manual_threshold = 0;
    if (argc > 1) {
        manual_threshold = strtoull(argv[1], NULL, 10);
    }

    buf = mmap(NULL, BUFF_SIZE, PROT_READ | PROT_WRITE,
               MAP_POPULATE | MAP_ANONYMOUS | MAP_PRIVATE | MAP_HUGETLB,
               -1, 0);

    if (buf == (void*) - 1) {
        perror("mmap() error\n");
        exit(EXIT_FAILURE);
    }

    *((char *)buf) = 1;

    for (int i = 0; i <= 8; i++) {
        build_set(i);
    }

    calibrate(manual_threshold);

    printf("Please press enter.\n");

    char text_buf[2];
    fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");

    while (1) {

        prime_set(8);

        for(volatile int k=0; k<20000; k++);

        uint64_t t8 = probe_set(8);

        if (t8 > thresholds[8]) {

            int bit_counts[8] = {0};
            int samples = 100;
            int valid_samples = 0;

            for (int s = 0; s < samples; s++) {

                for (int i = 0; i <= 8; i++)
                    prime_set(i);

                for(volatile int k=0; k<50000; k++);

                uint64_t t_valid = probe_set(8);

                if (t_valid > thresholds[8]) {

                    valid_samples++;

                    for (int i = 0; i < 8; i++) {
                        if (probe_set(i) > thresholds[i]) {
                            bit_counts[i]++;
                        }
                    }
                }
            }

            if (valid_samples > (samples * 0.5)) {

                int received_byte = 0;

                for (int i = 0; i < 8; i++) {

                    if (bit_counts[i] > (valid_samples * 0.75)) {
                        received_byte |= (1 << i);
                    }
                }

                printf("%d\n", received_byte);

                int consecutive_lows = 0;
                int timeout = 0;

                while(1) {

                    prime_set(8);

                    for(volatile int k=0; k<50000; k++);

                    if (probe_set(8) < thresholds[8])
                        consecutive_lows++;
                    else
                        consecutive_lows = 0;

                    if (consecutive_lows >= 10)
                        break;

                    timeout++;

                    if (timeout > 2000000)
                        break;
                }
            }
        }
    }

    return 0;
}
