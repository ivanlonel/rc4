/*
 * Adapted from http://wwww.cypherspace.org/adam/rsa/rc4.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define buf_size 1024

typedef struct rc4_key {
        uint8_t state[256];       
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
    for(counter = 0; counter < 256; counter++)
        state[counter] = counter;

    for(counter = 0; counter < 256; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
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
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % 256;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}

int main(int argc, char *argv[]) {
    FILE *input, *output;
    uint8_t buf[buf_size];
    uint8_t seed[256];
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
        fprintf(stderr, "Error: Input file %s not found.\n", argv[2]);
        return errno;
    }
    output = argc > 3 ? fopen(argv[3], "wb") : stdout;
    if (!output) {
        if (input)
            fclose(input);
        fprintf(stderr, "Error: Invalid output file.\n");
        return errno;
    }

    rd = fread(buf, 1, buf_size, input);
    while (rd > 0) {
        rc4(buf, rd, &key);
        fwrite(buf, 1, rd, output);
        rd = fread(buf, 1, buf_size, input);
    }

    fclose(input);
    if (argc > 3)
        fclose(output);

    return 0;
}
