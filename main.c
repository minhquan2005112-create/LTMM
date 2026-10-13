
#include <stdio.h>
#include <stddef.h>
#include "rabbit.h"

#define KEY_SIZE 16
#define BLOCK_SIZE 16
#define NUM_BLOCKS 32

void print_block(const char *name, const byte *data)
{
    printf("%s = [", name);

    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%02X", data[i]);

        if (i < BLOCK_SIZE - 1) {
            printf(" ");
        }
    }

    printf("]\n");
}

int main(void)
{
    byte key[KEY_SIZE] = {0};

    byte input[NUM_BLOCKS * BLOCK_SIZE] = {0};
    byte output[NUM_BLOCKS * BLOCK_SIZE] = {0};

    t_instance rabbit;

    key_setup(&rabbit, key);
    cipher(&rabbit, input, output, sizeof(input));

    print_block("s[0]", output);
    print_block("s[1]", output + BLOCK_SIZE);
    print_block("s[31]", output + 31 * BLOCK_SIZE);

    return 0;
}
