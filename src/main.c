#include "rc4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    rc4_key_t key;
    byte_t buf[BUF_SIZE];
    byte_t seed[SEED_SIZE];
    FILE *input, *output;
    size_t i, n;
    uint_fast16_t hex;
    char data[SEED_SIZE * 2 + 1] = "";
    /*char digit[3] = "00";
    char *p;*/

    if (argc < 2) {
        fprintf(stderr, "Syntax: %s hexadecimal_seed [input_file [output_file]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*If the data array length is even, truncate input key to 2 digits short of sizeof(data),
     *accounting for the null terminator and the eventual extra digit to even the number of digits.
     *If it's odd, reserve only the last position for the null terminator. */
    strncat(data, argv[1], sizeof data / sizeof data[0] - 2 + (sizeof data & 1));
    if (data[strspn(data, "0123456789abcdefABCDEF")] != '\0') {
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
        (void) sscanf(data + i * 2, "%2" SCNxFAST16, &hex);
        seed[i] = (byte_t) hex;
        /*Alternative way, without using inttypes.h:
        strncpy(digit, data + i * 2, 2);
        seed[i] = (byte_t) strtoul(digit, &p, 16);*/
    }

    prepare_key(seed, n, &key);

    input = argc > 2 ? fopen(argv[2], "rb") : freopen(NULL, "rb", stdin);
    if (!input) {
        perror("Couldn't open input stream");
        return EXIT_FAILURE;
    }
    output = argc > 3 ? fopen(argv[3], "wb") : freopen(NULL, "wb", stdout);
    if (!output) {
        perror("Couldn't open output stream");
        if (fclose(input) == EOF)
            perror("Couldn't close input stream");
        return EXIT_FAILURE;
    }

    /*while (!feof(input)) {
        i = fread(buf, sizeof buf[0], sizeof buf / sizeof buf[0], input);
        rc4(buf, i, &key);
        (void) fwrite(buf, sizeof buf[0], i, output);
    }*/
    i = fread(buf, sizeof buf[0], sizeof buf / sizeof buf[0], input);
    while (i > 0) {
        rc4(buf, i, &key);
        (void) fwrite(buf, sizeof buf[0], i, output);
        i = fread(buf, sizeof buf[0], sizeof buf / sizeof buf[0], input);
    }

    if (ferror(input))
        perror("Error reading from input stream");
    if (ferror(output))
        perror("Error writing to output stream");

    if (fclose(input) == EOF)
        perror("Couldn't close input stream");
    if (fclose(output) == EOF)
        perror("Couldn't close output stream");

    return EXIT_SUCCESS;
}
