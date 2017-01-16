#ifndef RC4_H
#define RC4_H

#include <stddef.h> /* size_t */

#if !defined (__GNUC__) && !defined (__restrict)
# if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define __restrict restrict
# else
#  define __restrict
# endif
#endif

#define SEED_SIZE 256

typedef unsigned char byte_t;

typedef struct rc4_key {
    byte_t state[SEED_SIZE];
    byte_t x;
    byte_t y;
} rc4_key_t;

rc4_key_t prepare_key (const byte_t *__restrict, size_t);

void rc4 (byte_t *__restrict, size_t, rc4_key_t *__restrict);

#endif /* RC4_H */
