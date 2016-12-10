//Adapted from http://www.cypherspace.org/adam/rsa/rc4.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define buf_size 1024

typedef struct rc4_key {
        unsigned char state[256];       
        unsigned char x;        
        unsigned char y;
} rc4_key;

void swap_byte(unsigned char *x, unsigned char *y) {
    unsigned char t = *x;
    *x = *y;
    *y = t;
}

void prepare_key(unsigned char *key_data_ptr, int key_data_len, rc4_key *key) {
    unsigned char index1;
    unsigned char index2;
    unsigned char* state;
    short counter;

    state = &key->state[0];
    for(counter = 0; counter < 256; counter++)
    state[counter] = counter;
    key->x = 0;
    key->y = 0;
    index1 = 0;
    index2 = 0;
    for(counter = 0; counter < 256; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key *key) {
    unsigned char x;
    unsigned char y;
    unsigned char* state;
    unsigned char xorIndex;
    short counter;

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

int main(int argc, char* argv[]) {
    unsigned char buf[buf_size];
    unsigned char seed[256];
    char data[512];
    char digit[5];
    unsigned int hex;
    int n, i, rd;
    FILE *input, *output;
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
        sscanf(digit, "%x", &hex);
        seed[i] = hex;
    }

    prepare_key(seed, n, &key);

    input = fopen(argv[2], "rb");
    output = argc > 3 ? fopen(argv[3], "wb") : stdout;

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

