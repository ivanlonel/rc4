#ifndef RC4_H
#define RC4_H

#include <stddef.h> /* size_t */

#if !defined (__GNUC__) && !defined (__restrict__)
# if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define __restrict__ restrict
# else
#  define __restrict__
# endif
#endif

#define SEED_SIZE 256

typedef unsigned char byte_t;

typedef struct rc4_key {
    byte_t x;
    byte_t y;
    byte_t state[SEED_SIZE];
} rc4_key_t;

void prepare_key(const byte_t *__restrict__, const size_t, rc4_key_t *__restrict__);

void rc4(byte_t *__restrict__, const size_t, rc4_key_t *__restrict__);

#endif /* RC4_H */
