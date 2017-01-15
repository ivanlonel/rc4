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
#endif /* defined (_WIN32) */

#ifndef SIZE_MAX /* stdint.h */
# define SIZE_MAX ((size_t) -1)
#endif

#ifndef CHAR_BIT /* limits.h */
# define CHAR_BIT 8
#endif

#define HB (CHAR_BIT / 4) /* Number of hexadecimal digits that fit in one byte. */

#define BUF_SIZE 0x8000u /* 32768u */

/**
 * @param str A string representation of an unsigned hexadecimal number. Must NOT contain "0x", "+" or "-".
 * @param length Maximum number of characters to be parsed from str.
 * @param arr Pointer to starting memory position to receive the value from str represented as a sequence of bytes.
 * @return The number of bytes used to represent the value from str.
 */
static size_t hex_str_to_byte_array (const char *__restrict__ str, size_t length, byte_t *__restrict__ arr) {
    size_t i, j;
    unsigned long hex;
    byte_t str_offset;
    byte_t arr_offset = 0;
    char buf[HB * sizeof hex + 1] = "";
    char *str_ptr;
    int temp_err = errno; /* Recommended type is errno_t starting on ISO C11 */

    if (length > strlen(str)) {
        length = strlen(str);
    }

    if (strspn(str, "0123456789abcdefABCDEF") < length) { /* No 0x, +, -, or whitespaces allowed. */
        errno = EINVAL;
        return SIZE_MAX - 1; /* str contains non-hexadecimal characters. */
    }

    errno = 0;

    if ((str_offset = length % (HB * sizeof hex)) != 0) {
		hex = strtoul(strncat(buf, str, str_offset), &str_ptr, 16);
        if (errno != 0) {
            return SIZE_MAX;
        }
        arr_offset = (byte_t) (str_offset / HB + !!(str_offset % HB)); 
        for (i = 0; i < arr_offset; i++) {
            arr[arr_offset - i - 1] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *arr));
        }
    }

    for (j = 0; j < length / (HB * sizeof hex); j++) {
        buf[0] = '\0';
        hex = strtoul(strncat(buf, str + str_offset + j * (HB * sizeof hex), HB * sizeof hex), &str_ptr, 16);
        if (errno != 0) {
            return SIZE_MAX;
        }
        for (i = 0; i < sizeof hex; i++) {
            arr[arr_offset + j * sizeof hex + sizeof hex - i - 1] = (byte_t) (hex >> (i * CHAR_BIT * sizeof *arr));
        }
    }

    errno = temp_err;
    return arr_offset + j * sizeof hex; /* j == length / (HB * sizeof hex) */
}

int main(int argc, char *argv[]) {
    rc4_key_t key;
    byte_t buf[BUF_SIZE],
           seed[SEED_SIZE];
    FILE *input,
         *output;
    size_t length;

    if (argc < 2) {
        (void) fprintf(stderr, "Usage: %s hexadecimal_seed [input_file [output_file]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strlen(argv[1]) > HB * SEED_SIZE) {
        (void) fprintf(stderr, "Seed will be truncated to %d digits.\n", HB * SEED_SIZE);
    }

    errno = 0;
	
	length = hex_str_to_byte_array(argv[1], HB * SEED_SIZE, seed);
	
    if (length == SIZE_MAX) {
        perror("Couldn't parse seed");
        return EXIT_FAILURE;
    } else if (length == SIZE_MAX - 1) {
        perror("Key contains non-hexadecimal characters");
        return EXIT_FAILURE;
    }

    prepare_key(seed, length, &key);

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
        length = fread(buf, sizeof *buf, BUF_SIZE, input);
        rc4(buf, length, &key);
    } while (fwrite(buf, sizeof *buf, length, output) == BUF_SIZE);
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

    if (fclose(input) == EOF) {
        perror("Couldn't close input stream");
    }
    if (fclose(output) == EOF) {
        perror("Couldn't close output stream");
    }

    return EXIT_SUCCESS;
}
