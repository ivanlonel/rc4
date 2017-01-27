#include "rc4.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Modify a standard I/O stream mode to binary, since freopen(NULL, ...) won't work on Windows. */
#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
# ifdef __STRICT_ANSI__
#  define FILE_NO(handle) (handle == stdin ? 0 : handle == stdout ? 1 : handle == stderr ? 2 : -1)
# else
#  define FILE_NO(handle) _fileno(handle)
# endif /* defined __STRICT_ANSI__ */
# define SET_BINARY_MODE(handle) (_setmode(FILE_NO(handle), _O_BINARY) == -1 ? NULL : handle)
#else /* Other operating systems will not care about the difference between text and binary modes. */
# define SET_BINARY_MODE(handle) handle
#endif /* defined _WIN32 */

#ifndef SIZE_MAX /* stdint.h */
# define SIZE_MAX ((size_t) -1)
#endif

#ifndef CHAR_BIT /* limits.h */
# define CHAR_BIT 8
#endif

#define HEXB (CHAR_BIT / 4) /* Number of hexadecimal digits that fit in one byte. */

/* Make sure this is a power of 2. It shouldn't be greater than half the size of the target's L1 data cache,
 * unless the reduced disk I/O overhead of a greater buffer size makes up for the cache misses somehow. */
#define BUF_SIZE 8192u

/**
 * @param str A string representation of an unsigned hexadecimal number. Must NOT contain "0x", "+" or "-".
 * @param length Maximum number of characters to be parsed from str.
 * @param arr Pointer to starting memory position to receive the value from str represented as an array of bytes.
 * @return The number of bytes used to store the value from str. SIZE_MAX or SIZE_MAX-1 on error (sets errno).
 */
static size_t hex_str_to_byte_array (byte_t *__restrict arr, const char *__restrict str, size_t length) {
    size_t i, j;
    unsigned long hex;
    char **__restrict str_ptr = NULL;
    char buf[HEXB * sizeof hex + 1] = "";
    byte_t str_offset;
    byte_t offset = 0;
    int temp_err = errno; /* Recommended type is errno_t starting on ISO C11 */

    if (strlen(str) < length) {
        length = strlen(str);
    }

    if (strspn(str, "0123456789abcdefABCDEF") < length) { /* No 0x, +, -, or whitespaces accepted. */
        errno = EINVAL;
        return SIZE_MAX - 1; /* str contains non-hexadecimal characters. */
    }

    errno = 0;

    str_offset = length % (HEXB * sizeof hex);
    if (str_offset) {
        hex = strtoul(strncat(buf, str, str_offset), str_ptr, 16);
        if (errno) {
            return SIZE_MAX;
        }
        offset = (byte_t) (str_offset / HEXB + !!(str_offset % HEXB)); 
        for (i = 0; i < offset; i++) {
            arr[offset - i - 1] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *arr));
        }
    }

    for (j = 0; j < length / (HEXB * sizeof hex); j++) {
        buf[0] = '\0';
        hex = strtoul(strncat(buf, str + str_offset + j * (HEXB * sizeof hex), HEXB * sizeof hex), str_ptr, 16);
        if (errno) {
            return SIZE_MAX;
        }
        for (i = 0; i < sizeof hex; i++) {
            arr[offset + j * sizeof hex + (sizeof hex - 1) - i] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *arr));
        }
    }

    errno = temp_err;
    return offset + j * sizeof hex; /* j == length / (HEXB * sizeof hex) */
}

int main (int argc, char **__restrict argv) {
    byte_t buf[BUF_SIZE];
    byte_t seed[SEED_SIZE];
    rc4_key_t key;
    FILE *__restrict input;
    FILE *__restrict output;
    size_t length;

    if (2 > argc) {
        (void) fprintf(stderr, "Usage: %s hexadecimal_seed [input_file [output_file]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (HEXB * SEED_SIZE < strlen(argv[1])) {
        (void) fprintf(stderr, "Seed should be %d digits or less. Entry will be truncated.\n", HEXB * SEED_SIZE);
    }

    errno = 0;
    length = hex_str_to_byte_array(seed, argv[1], HEXB * SEED_SIZE);
    if (SIZE_MAX == length) {
        perror("Couldn't parse seed");
        return EXIT_FAILURE;
    } else if (SIZE_MAX - 1 == length) {
        perror("Seed contains non-hexadecimal characters");
        return EXIT_FAILURE;
    }

    key = prepare_key(seed, length);

    errno = 0;
    input = 2 < argc ? fopen(argv[2], "rb") : SET_BINARY_MODE(stdin);
    if (!input) {
        perror("Couldn't open input stream");
        return EXIT_FAILURE;
    }

    errno = 0;
    output = 3 < argc ? fopen(argv[3], "wb") : SET_BINARY_MODE(stdout);
    if (!output) {
        perror("Couldn't open output stream");
        errno = 0;
        if (EOF == fclose(input)) {
            perror("Couldn't close input stream");
        }
        return EXIT_FAILURE;
    }

    errno = 0;
    do {
        length = fread(buf, sizeof *buf, BUF_SIZE, input);
        rc4(buf, length, &key);
    } while (BUF_SIZE == fwrite(buf, sizeof *buf, length, output));
#if 0
    while (!feof(input)) { /* disabled code */
        length = fread(buf, sizeof *buf, BUF_SIZE, input);
        rc4(buf, length, &key);
        (void) fwrite(buf, sizeof *buf, length, output);
    }
#endif
    if (ferror(input)) {
        perror("Error reading from input stream");
    }
    if (ferror(output)) {
        perror("Error writing to output stream");
    }

    errno = 0;
    if (EOF == fclose(input)) {
        perror("Couldn't close input stream");
    }
    errno = 0;
    if (EOF == fclose(output)) {
        perror("Couldn't close output stream");
    }

    return EXIT_SUCCESS;
}
