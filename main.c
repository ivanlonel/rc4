#include "rc4.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

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
        return EXIT_FAILURE;
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
