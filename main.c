#include "rc4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    rc4_key_t key;
    byte_t buf[BUF_SIZE];
    byte_t seed[SEED_SIZE];
    FILE *input, *output;
    size_t i, n;
    uint_fast16_t hex;
    char data[SEED_SIZE * 2] = "";
    /*char digit[3] = "00";
    char *p;*/

    if (argc < 3) {
        fprintf(stderr, "Syntax: %s hexadecimal_seed input_file [output_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*Truncating input key to 2 digits short of sizeof(data), accounting for the null terminator
     * and the eventual extra digit to even the number of digits */
    strncat(data, argv[1], sizeof(data) - 2);
    if (data[strspn(data, "0123456789abcdefABCDEF")] != 0) {
        fprintf(stderr, "Key \"%s\" contains non-hexadecimal characters.\n", argv[1]);
        return EXIT_FAILURE;
    }

    /*Making sure the raw seed has an even number of hexadecimal digits by appending '0' at the end.*/
    n = strlen(data);
    if (n & 1) {
        data[n++] = '0';
        data[n] = '\0';
    }
    n /= 2;

    /*Converting hexadecimal char string to byte array.*/
    for (i = 0; i < n; i++) {
        sscanf(data + i * 2, "%2"SCNxFAST16, &hex);
        seed[i] = (byte_t) hex;
        /*//Alternative way, without using inttypes.h:
        strncpy(digit, data + i * 2, 2);
        seed[i] = (byte_t) strtoul(digit, &p, 16);*/
    }

    prepare_key(seed, n, &key);

    input = fopen(argv[2], "rb");
    if (!input) {
        fprintf(stderr, "Couldn't open input file \"%s\": %s.\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    }
    output = argc > 3 ? fopen(argv[3], "wb") : stdout;
    if (!output) {
        perror("Couldn't open output stream");
        fclose(input);
        return EXIT_FAILURE;
    }

    i = fread(buf, 1, BUF_SIZE, input);
    while (i > 0) {
        rc4(buf, i, &key);
        fwrite(buf, 1, i, output);
        i = fread(buf, 1, BUF_SIZE, input);
    }

    fclose(input);
    if (argc > 3)
        fclose(output);

    return EXIT_SUCCESS;
}
