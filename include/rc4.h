#ifndef RC4_H
#define RC4_H

#include <stdlib.h>

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
# include <stdint.h>
# if UINT_FAST8_MAX < UINTMAX_MAX
#  define SEED_SIZE (UINT_FAST8_MAX + 1u)
# else
#  define SEED_SIZE UINT8_C(256)
# endif
  typedef uint_fast8_t byte_t;
#else
# include <limits.h>
# if UCHAR_MAX < ULONG_MAX
#  define SEED_SIZE (UCHAR_MAX + 1u)
# else
#  define SEED_SIZE 256u
# endif
  typedef unsigned char byte_t;
#endif /* __STDC_VERSION__ >= 199901L */

typedef struct rc4_key {
    byte_t state[SEED_SIZE];
    byte_t i;
    byte_t j;
} rc4_key_t;

#if !defined __restrict && !defined __GNUC__ && !(defined _MSC_VER && _MSC_VER >= 1400)
# if defined __restrict__
#  define __restrict __restrict__
# elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  define __restrict restrict
# else
#  define __restrict
# endif
#endif

rc4_key_t prepare_key (const byte_t *__restrict, size_t);

void rc4 (byte_t *__restrict, size_t, rc4_key_t *__restrict);

#endif /* RC4_H */
