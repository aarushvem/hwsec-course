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

void evict_set(int set_index) {
    struct node *curr = sets[set_index];
    while (curr) curr = curr->next;
    curr = sets[set_index];
    while (curr) curr = curr->next;
}

int main(int argc, char **argv)
{
  srand(time(NULL));

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

  bool sending = true;
  while (sending) {
      printf("Please type a message.\n");
	  
      char text_buf[128];
      if (fgets(text_buf, sizeof(text_buf), stdin) == NULL) break;

      int message = atoi(text_buf);

      long duration = 4000000;

      for (long k = 0; k < duration; k++) {

          evict_set(8);

          for (int i = 0; i < 8; i++) {
              if ((message >> i) & 1) {
                  evict_set(i);
              }
          }
      }
  }

  printf("Sender finished.\n");
  return 0;
}
