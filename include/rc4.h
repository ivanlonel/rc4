#ifndef RC4_H
#define RC4_H

#include <stdlib.h>

#if !defined __restrict && !defined __GNUC__ && !(defined _MSC_VER && _MSC_VER >= 1400)
# if defined __restrict__
#  define __restrict __restrict__
# elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  define __restrict restrict
# else
#  define __restrict
# endif
#endif

#define SEED_SIZE 256

typedef unsigned char byte_t;

typedef struct rc4_key {
    byte_t state[SEED_SIZE];
    byte_t i;
    byte_t j;
} rc4_key_t;

rc4_key_t prepare_key (const byte_t *__restrict, size_t);

void rc4 (byte_t *__restrict, size_t, rc4_key_t *__restrict);

#endif /* RC4_H */
