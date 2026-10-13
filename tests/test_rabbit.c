#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../rabbit.h"

#define BLOCK_SIZE 16
#define KEY_SIZE 16
#define NUM_BLOCKS 32

// Chuyen chuoi hex thanh mang byte
int hexToBytes(const char *hex, byte *out, int n)
{
    if (strlen(hex) != (size_t)(n * 2))
        return 0;

    for (int i = 0; i < n; i++) {
        unsigned int value;

        if (sscanf(hex + 2 * i, "%2x", &value) != 1)
            return 0;

        out[i] = (byte)value;
    }

    return 1;
}

// So sanh hai mang byte
int compareBytes(const byte *actual, const byte *expected, int n)
{
    return memcmp(actual, expected, n) == 0;
}

int main(void)
{
    FILE *f = fopen("tests/rabbit_vectors.txt", "r");

    if (f == NULL) {
        printf("Khong mo duoc tests/rabbit_vectors.txt\n");
        return 1;
    }

    char line[512];
    int testNumber = 0;
    int passed = 0;
    int failed = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        char keyHex[33], s0Hex[33], s1Hex[33], s31Hex[33];

        if (line[0] == '#' || line[0] == '\n')
            continue;

        if (sscanf(line, "%32s %32s %32s %32s",
                   keyHex, s0Hex, s1Hex, s31Hex) != 4) {
            printf("Dong du lieu khong hop le: %s", line);
            failed++;
            continue;
        }

        byte key[KEY_SIZE];
        byte input[NUM_BLOCKS * BLOCK_SIZE] = {0};
        byte output[NUM_BLOCKS * BLOCK_SIZE] = {0};

        byte expected0[BLOCK_SIZE];
        byte expected1[BLOCK_SIZE];
        byte expected31[BLOCK_SIZE];

        if (!hexToBytes(keyHex, key, KEY_SIZE) ||
            !hexToBytes(s0Hex, expected0, BLOCK_SIZE) ||
            !hexToBytes(s1Hex, expected1, BLOCK_SIZE) ||
            !hexToBytes(s31Hex, expected31, BLOCK_SIZE)) {
            printf("Test vector co chuoi hex khong hop le\n");
            failed++;
            continue;
        }

        t_instance rabbit;
        key_setup(&rabbit, key);
        cipher(&rabbit, input, output, sizeof(input));

        testNumber++;

        int ok0 = compareBytes(output, expected0, BLOCK_SIZE);
        int ok1 = compareBytes(output + BLOCK_SIZE, expected1, BLOCK_SIZE);
        int ok31 = compareBytes(output + 31 * BLOCK_SIZE,
                                expected31, BLOCK_SIZE);

        if (ok0 && ok1 && ok31) {
            printf("Test vector %d: PASS\n", testNumber);
            passed++;
        } else {
            printf("Test vector %d: FAIL", testNumber);

            if (!ok0) printf(" (s[0] sai)");
            if (!ok1) printf(" (s[1] sai)");
            if (!ok31) printf(" (s[31] sai)");

            printf("\n");
            failed++;
        }
    }

    fclose(f);

    printf("\nTong ket: %d PASS, %d FAIL\n", passed, failed);

    return failed == 0 && testNumber > 0 ? 0 : 1;
}