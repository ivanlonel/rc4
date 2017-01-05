#include "rc4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Modify a standard I/O stream mode to binary, since freopen(NULL, ...) doesn't work on Windows.*/
#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
# define FILE_NO(handle) (handle == stdin ? 0 : handle == stdout ? 1 : handle == stderr ? 2 : -1)
# define SET_BINARY_MODE(handle) (_setmode(FILE_NO(handle), _O_BINARY) == -1 ? NULL : handle)
#else
# define SET_BINARY_MODE(handle) handle
#endif

#define BUF_SIZE 0x8000u /*32768u*/

int main(int argc, char *argv[]) {
    rc4_key_t key;
    byte_t buf[BUF_SIZE];
    byte_t seed[SEED_SIZE];
    FILE *input, *output;
    size_t i, n;
    long unsigned hex;
    char data[SEED_SIZE * 2 + 1] = "";
    /*char digit[3] = "00";
    char *p;*/

    if (argc < 2) {
        fprintf(stderr, "Usage: %s hexadecimal_seed [input_file [output_file]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*If the data array length is even, truncate input key to 2 digits short of sizeof(data),
     *accounting for the null terminator and the eventual extra digit to even the number of digits.
     *If it's odd, reserve only the last position for the null terminator. */
    n = strlen(strncat(data, argv[1], sizeof data - 2 + (sizeof data & 1)));

    if (n < strlen(argv[1])) {
        fprintf(stderr, "Seed should be %lu digits or less. Entry's been truncated.\n", (long unsigned) n);
        /*fprintf(stderr, "Seed should be %zu digits or less. Entry's been truncated.\n", n);*/ /*ISO C99*/
    }

    if (data[strspn(data, "0123456789abcdefABCDEF")] != '\0') {
        fprintf(stderr, "Key \"%s\" contains non-hexadecimal characters.\n", data);
        return EXIT_FAILURE;
    }

    /*Making sure the raw seed has an even number of hexadecimal digits by appending '0' at the end.*/
    if (n & 1u) {
        data[n++] = '0';
        data[n] = '\0';
    }
    n /= 2;

    /*Converting hexadecimal char string to byte array.*/
    for (i = 0; i < n; i++) {
        /*if ((hex = strtoul(strncpy(digit, data + i * 2, 2), &p, 16)) == -1LU) {*/ /*Alternative*/
        if (sscanf(data + i * 2, "%2lx", &hex) == EOF) {
            perror("Couldn't parse seed");
            return EXIT_FAILURE;
        }

        seed[i] = (byte_t) hex;
    }

    prepare_key(seed, n, &key);

    input = argc > 2 ? fopen(argv[2], "rb") : SET_BINARY_MODE(stdin);
    if (!input) {
        perror("Couldn't open input stream");
        return EXIT_FAILURE;
    }
    output = argc > 3 ? fopen(argv[3], "wb") : SET_BINARY_MODE(stdout);
    if (!output) {
        perror("Couldn't open output stream");
        if (fclose(input) == EOF) {
            perror("Couldn't close input stream");
        }
        return EXIT_FAILURE;
    }

    do {
        i = fread(buf, sizeof buf[0], BUF_SIZE, input);
        rc4(buf, i, &key);
    } while (fwrite(buf, sizeof buf[0], i, output) == BUF_SIZE);
    /*while (!feof(input)) {
        i = fread(buf, sizeof buf[0], BUF_SIZE, input);
        rc4(buf, i, &key);
        fwrite(buf, sizeof buf[0], i, output);
    }*/ /*Alternative*/

    if (ferror(input)) {
        perror("Error reading from input stream");
    }
    if (ferror(output)) {
        perror("Error writing to output stream");
    }

    if (fclose(input) == EOF) {
        perror("Couldn't close input stream");
    }
    if (fclose(output) == EOF) {
        perror("Couldn't close output stream");
    }

    return EXIT_SUCCESS;
}
