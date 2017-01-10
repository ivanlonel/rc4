#include "rc4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#ifdef _WIN32 /* Modify a standard I/O stream mode to binary, since freopen(NULL, ...) doesn't work on Windows. */
# include <io.h>
# include <fcntl.h>
# ifdef __STRICT_ANSI__
#  define FILE_NO(handle) (handle == stdin ? 0 : handle == stdout ? 1 : handle == stderr ? 2 : -1)
# else
#  define FILE_NO(handle) _fileno(handle)
# endif /* defined __STRICT_ANSI__ */
# define SET_BINARY_MODE(handle) (_setmode(FILE_NO(handle), _O_BINARY) == -1 ? NULL : handle)
#else
# define SET_BINARY_MODE(handle) handle
#endif /* defined _WIN32 */

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

#ifndef ULONG_MAX
# define ULONG_MAX (-1LU)
#endif

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#define BUF_SIZE 0x8000u /*32768u*/

static size_t hex_str_to_byte_array (const char *__restrict__ str, size_t length, byte_t *__restrict__ array) {
    size_t i, j;
    byte_t str_offset;
    byte_t arr_offset = 0;
    long unsigned hex;
    char buf[2 * sizeof hex + 1] = "";
    char *str_ptr;
    int temp_err = errno;

    errno = 0;

    if (length > strlen(str)) {
        length = strlen(str);
    }

    if ((str_offset = length % (2 * sizeof hex)) != 0) {
        if ((hex = strtoul(strncat(buf, str, str_offset), &str_ptr, 16)) == ULONG_MAX || errno != 0) {
            return SIZE_MAX; /* (str_offset < sizeof hex) implies (hex < ULONG_MAX) unless strtoul has failed. */
        }
        arr_offset = (byte_t) (str_offset / 2 + str_offset % 2);
        for (i = 0; i < arr_offset; i++) {
            array[arr_offset - i - 1] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *array));
        }
    }

    for (j = 0; j < length / (2 * sizeof hex); j++) {
        buf[0] = '\0';
        hex = strtoul(strncat(buf, str + str_offset + j * (2 * sizeof hex), 2 * sizeof hex), &str_ptr, 16);
        if (errno != 0) {
            return SIZE_MAX;
        }
        for (i = 0; i < sizeof hex; i++) {
            array[arr_offset + j * sizeof hex + sizeof hex - i - 1] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *array));
        }
    }

    errno = temp_err;
    return arr_offset + j * sizeof hex;
}

int main(int argc, char *argv[]) {
    rc4_key_t key;
    byte_t buf[BUF_SIZE],
           seed[SEED_SIZE];
    FILE *input,
         *output;
    size_t n;
    char data[2 * SEED_SIZE + 1] = "";

    if (argc < 2) {
        (void) fprintf(stderr, "Usage: %s hexadecimal_seed [input_file [output_file]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    n = strlen(strncat(data, argv[1], sizeof data - 1));

    if (n < strlen(argv[1])) {
#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
        (void) fprintf(stderr, "Seed should be %zu digits or less. Entry's been truncated.\n", n);
#else
        (void) fprintf(stderr, "Seed should be %lu digits or less. Entry's been truncated.\n", (long unsigned) n);
#endif
    }

    if (data[strspn(data, "0123456789abcdefABCDEF")] != '\0') {
        (void) fprintf(stderr, "Key \"%s\" contains non-hexadecimal characters.\n", data);
        return EXIT_FAILURE;
    }

    errno = 0;
    if ((n = hex_str_to_byte_array(data, n, seed)) == SIZE_MAX) {
        perror("Couldn't parse seed");
        return EXIT_FAILURE;
    }

    prepare_key(seed, n, &key);

    errno = 0;
    input = argc > 2 ? fopen(argv[2], "rb") : SET_BINARY_MODE(stdin);
    if (!input) {
        perror("Couldn't open input stream");
        return EXIT_FAILURE;
    }

    errno = 0;
    output = argc > 3 ? fopen(argv[3], "wb") : SET_BINARY_MODE(stdout);
    if (!output) {
        perror("Couldn't open output stream");
        errno = 0;
        if (fclose(input) == EOF) {
            perror("Couldn't close input stream");
        }
        return EXIT_FAILURE;
    }

    errno = 0;
    do {
        n = fread(buf, sizeof *buf, BUF_SIZE, input);
        rc4(buf, n, &key);
    } while (fwrite(buf, sizeof *buf, n, output) == BUF_SIZE);
#if 0
    while (!feof(input)) { /* disabled code */
        n = fread(buf, sizeof *buf, BUF_SIZE, input);
        rc4(buf, n, &key);
        (void) fwrite(buf, sizeof *buf, n, output);
    }
#endif

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
