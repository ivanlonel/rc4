/*
 * Adapted from http://wwww.cypherspace.org/adam/rsa/rc4.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#define BUF_SIZE 1024
#define SEED_SIZE 256

typedef struct rc4_key {
    uint8_t state[SEED_SIZE];
    uint8_t x;        
    uint8_t y;
} rc4_key;

void swap_byte(uint8_t *x, uint8_t *y) {
    uint8_t t = *x;
    *x = *y;
    *y = t;
}

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len, rc4_key *key) {
    uint8_t *state;
    uint8_t index1 = 0;
    uint8_t index2 = 0;
    size_t counter;

    key->x = 0;
    key->y = 0;

    state = &key->state[0];
    for(counter = 0; counter < SEED_SIZE; counter++)
        state[counter] = counter;

    for(counter = 0; counter < SEED_SIZE; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % SEED_SIZE;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void rc4(uint8_t *buffer_ptr, size_t buffer_len, rc4_key *key) {
    uint8_t x;
    uint8_t y;
    uint8_t *state;
    uint8_t xorIndex;
    size_t counter;

    x = key->x;
    y = key->y;
    state = &key->state[0];
    for(counter = 0; counter < buffer_len; counter++) {
        x = (x + 1) % SEED_SIZE;
        y = (state[x] + y) % SEED_SIZE;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % SEED_SIZE;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}

int main(int argc, char *argv[]) {
    FILE *input, *output;
    uint8_t buf[BUF_SIZE];
    uint8_t seed[SEED_SIZE];
    char data[512];
    char digit[5];
    uint16_t hex;
    size_t i, n, rd;
    rc4_key key;

    if (argc < 3) {
        fprintf(stderr, "Syntax: %s key in_file [out_file]\n", argv[0]);
        exit(1);
    }

    strcpy(data, argv[1]);
    n = strlen(data);
    if (n & 1) {
        strcat(data, "0");
        n++;
    }
    n /= 2;

    strcpy(digit, "AA");
    digit[4] = '\0';
    for (i = 0; i < n; i++) {
        digit[2] = data[i * 2];
        digit[3] = data[i * 2 + 1];
        sscanf(digit, "%"SCNu16, &hex);
        seed[i] = hex;
    }

    prepare_key(seed, n, &key);

    input = fopen(argv[2], "rb");
    if (!input) {
        fprintf(stderr, "Couldn't open input file \"%s\": %s.\n", argv[2], strerror(errno));
        return errno;
    }
    output = argc > 3 ? fopen(argv[3], "wb") : stdout;
    if (!output) {
        perror("Couldn't open output stream");
        if (input)
            fclose(input);
        return errno;
    }

    rd = fread(buf, 1, BUF_SIZE, input);
    while (rd > 0) {
        rc4(buf, rd, &key);
        fwrite(buf, 1, rd, output);
        rd = fread(buf, 1, BUF_SIZE, input);
    }

    fclose(input);
    if (argc > 3)
        fclose(output);

    return 0;
}
