#ifndef RC4_H
#define RC4_H

#include <stdlib.h>

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
# define RESTRICT_IF_SUPPORTED restrict
#else
# define RESTRICT_IF_SUPPORTED
#endif

#define SEED_SIZE 256

typedef unsigned char byte_t;

typedef struct rc4_key {
    byte_t x;
    byte_t y;
    byte_t state[SEED_SIZE];
} rc4_key_t;

void prepare_key(const byte_t *RESTRICT_IF_SUPPORTED, const size_t, rc4_key_t *RESTRICT_IF_SUPPORTED);

void rc4(byte_t *RESTRICT_IF_SUPPORTED, const size_t, rc4_key_t *RESTRICT_IF_SUPPORTED);

#endif /* RC4_H */
