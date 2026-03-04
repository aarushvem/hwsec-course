#include "utility.h"

// TODO: Uncomment the following lines and fill in the correct size
#define L1_SIZE 32768
#define L2_SIZE 1048576
#define L3_SIZE 11534336
 
int main (int ac, char **av) {

    // create 4 arrays to store the latency numbers
    // the arrays are initialized to 0
    uint64_t dram_latency[SAMPLES] = {0};
    uint64_t l1_latency[SAMPLES] = {0};
    uint64_t l2_latency[SAMPLES] = {0};
    uint64_t l3_latency[SAMPLES] = {0};

    // A temporary variable we can use to load addresses
    // The volatile keyword tells the compiler to not put this variable into a
    // register- it should always try to load from memory/ cache.
    volatile char tmp;

    // Allocate a buffer of 64 Bytes
    // the size of an unsigned integer (uint64_t) is 8 Bytes
    // Therefore, we request 8 * 8 Bytes
    uint64_t *target_buffer = (uint64_t *)malloc(8*sizeof(uint64_t));

    if (NULL == target_buffer) {
        perror("Unable to malloc");
        return EXIT_FAILURE;
    }

    // [1.2] TODO: Uncomment the following line to allocate a buffer of a size
    // of your chosing. This will help you measure the latencies at L2 and L3.
    uint8_t *eviction_buffer = (uint8_t *)malloc((L2_SIZE * 3) / 2);
if (NULL == eviction_buffer) {
    perror("Unable to malloc eviction_buffer");
    return EXIT_FAILURE;
}

    // Example: Measure L1 access latency, store results in l1_latency array
    for (int i=0; i<SAMPLES; i++){
        // Step 1: bring the target cache line into L1 by simply accessing the line
        tmp = target_buffer[0];

        // Step 2: measure the access latency
        l1_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
    }

    // ======
    // [1.2] TODO: Measure DRAM Latency, store results in dram_latency array
    // ======
    //
    for (int i = 0; i < SAMPLES; i++) {
    clflush((void*)target_buffer);  // force it out of cache
    dram_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
}
    // ======
    // [1.2] TODO: Measure L2 Latency, store results in l2_latency array
    // ======
    //
for (int i = 0; i < SAMPLES; i++) {
    // Step 1: bring target into L1 (and thus also in L2 underneath)
    tmp = ((volatile char*)target_buffer)[0];

    // Step 2: evict from L1 by touching ~1.5x L1 worth of other cache lines
    for (int off = 0; off < (L1_SIZE * 3) / 2; off += 64) {
        tmp ^= eviction_buffer[off];
    }

    // Optional extra pass (helps stability)
    for (int off = 0; off < (L1_SIZE * 3) / 2; off += 64) {
        tmp ^= eviction_buffer[off];
    }

    // Step 3: measure (should now be L2 hit more often)
    l2_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
}
    // ======
    // [1.2] TODO: Measure L3 Latency, store results in l3_latency array
    // ======
    //

    for (int i = 0; i < SAMPLES; i++) {
    // Step 1: bring target into cache hierarchy
    tmp = ((volatile char*)target_buffer)[0];

    // Step 2: evict from L2 by thrashing ~1.5x L2 buffer at cache-line stride
    for (int off = 0; off < (L2_SIZE * 3) / 2; off += 64) {
        tmp ^= eviction_buffer[off];
    }
    // Extra pass to help stability (lab suggests multiple passes) :contentReference[oaicite:2]{index=2}
    for (int off = 0; off < (L2_SIZE * 3) / 2; off += 64) {
        tmp ^= eviction_buffer[off];
    }

    // Step 3: measure (should be L3 hit more often)
    l3_latency[i] = measure_one_block_access_time((uint64_t)target_buffer);
}
    // Print the results to the screen
    // [1.5] Change print_results to print_results_for_python so that your code will work
    // with the python plotter software
    print_results_for_python(dram_latency, l1_latency, l2_latency, l3_latency);

    free(target_buffer);

    // [1.2] TODO: Uncomment this line once you uncomment the eviction_buffer creation line
    free(eviction_buffer);
    return 0;
}

