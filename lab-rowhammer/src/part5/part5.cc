#include "../ecc.hh"
#include "../verif.hh"

uint8_t parity_eqs[5][16] = {
    {1,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1},
    {1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0},
    {0,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1},
    {0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1}
};

uint32_t genParity(uint32_t data) {
    uint32_t parity = 0;
    for (int p = 0; p < 5; p++) {
        uint32_t bit = 0;
        for (int d = 0; d < NUM_DATA_BITS; d++) {
            if (parity_eqs[p][d]) {
                bit ^= getBit(data, d);
            }
        }
        parity |= (bit << p);
    }
    uint32_t p5 = 0;
    for (int d = 0; d < NUM_DATA_BITS; d++) p5 ^= getBit(data, d);
    for (int p = 0; p < 5; p++) p5 ^= getBit(parity, p);
    parity |= (p5 << 5);
    return parity;
}

struct hamming_result findHammingErrors(uint32_t encoded) {
    hamming_struct decoded = extractEncoding(encoded);
    uint32_t recordedParity = decoded.parity;
    uint32_t regenParity = genParity(decoded.data);
    uint32_t syndrome = (recordedParity ^ regenParity) & 0x1F;
    uint32_t overall_parity = 0;
    for (int i = 0; i < TOTAL_BITS; i++) overall_parity ^= getBit(encoded, i);
    uint32_t P5_Error_bit = overall_parity;
    _ERROR_TYPE error = NO_ERROR;
    if (syndrome == 0 && P5_Error_bit == 0) error = NO_ERROR;
    else if (syndrome != 0 && P5_Error_bit == 1) error = SINGLE_ERROR;
    else if (syndrome != 0 && P5_Error_bit == 0) error = DOUBLE_ERROR;
    else error = PARITY_ERROR;
    return {error, syndrome};
}

uint32_t verifyAndRepair(uint32_t encoded) {
    struct hamming_result result = findHammingErrors(encoded);
    uint32_t out = encoded;
    if (result.error == SINGLE_ERROR) {
        out = flipBit(encoded, result.syndrome - 1);
    } else if (result.error == PARITY_ERROR) {
        out = flipBit(encoded, TOTAL_BITS - 1);
    }
    return out;
}

/*
 *
 * DO NOT MODIFY BELOW ME
 *
 */

int main(void) {
    int numTests = 5;
    int pass = 1;
    for (int testIter = 0; testIter < numTests; testIter++) {
        printf("=== Test Iteration %d / %d ===\n", testIter+1, numTests);
        srand (time(NULL));
        uint32_t data = rand() % (1 << NUM_DATA_BITS);
        printf("Random Data Generated: %x\n", data);
        uint32_t parity = genParity(data);
        printf("Parity of Generated Data: %x\n", parity);
        uint32_t encoded = embedEncoding({data, parity});
        printf("Embedded Encoding: %x\n", encoded);
        if(!checkParity(encoded)) {
            printf("FAIL: Not quite! Your parity value is incorrect.\n");
            return -1;
        } else {
            printf("PASS: Your parity value is correct.\n");
        }
        printf("\nVerifying (no flips)...\n");
        if (findHammingErrors(encoded).error != NO_ERROR) {
            printf("FAIL: Error detected when there wasn't one!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded)) {
            printf("FAIL: verifyAndRepair changed a valid encoding!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        printf("\nVerifying (one flip)...\n");
        uint32_t encoded_oneflip = injectRandomFlips(encoded,1);
        printf("Data after one bit flip: %x\n", encoded_oneflip);
        uint32_t repaired = verifyAndRepair(encoded_oneflip);
        printf("Repaired Encoding: %x\n", repaired);
        if (findHammingErrors(encoded_oneflip).error != SINGLE_ERROR) {
            printf("FAIL: Single error not detected!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded_oneflip)) {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        printf("\nVerifying (parity flip)...\n");
        uint32_t encoded_parityflip = flipBit(encoded,TOTAL_BITS-1);
        printf("Data after parity bit flip: %x\n", encoded_parityflip);
        repaired = verifyAndRepair(encoded_parityflip);
        printf("Repaired Encoding: %x\n", repaired);
        if (findHammingErrors(encoded_parityflip).error != PARITY_ERROR) {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        } else if (encoded != verifyAndRepair(encoded_parityflip)) {
            printf("FAIL: verifyAndRepair failed to correct value!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        printf("\nVerifying (double flip)...\n");
        uint32_t encoded_twoflip = injectRandomFlips(encoded,2);
        printf("Two bit flip: %x\n", encoded_twoflip);
        if (findHammingErrors(encoded_twoflip).error != DOUBLE_ERROR) {
            printf("FAIL: Parity error not detected!\n");
            pass = 0;
        } else {
            printf("PASS.\n");
        }
        if (!pass) {
            printf("\n*** One or more tests failed. ***\n");
            break;
        }
    }
    if(pass) {
        printf("\n*** All tests passed, congratulations! ***\n");
    }
}
