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

#if !defined __GNUC__ && !(defined _MSC_VER && _MSC_VER >= 1400)
# if !defined __restrict
#  if defined __restrict__
#   define __restrict __restrict__
#  elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __restrict restrict
#  else
#   define __restrict
#  endif
# endif /* !defined __restrict */
# if !defined __inline && !(defined _MSC_VER && _MSC_VER >= 1310)
#  if defined __inline__
#   define __inline __inline__
#  elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __inline inline
#  else
#   define __inline
#  endif
# endif /* !defined __inline */
#endif /* !defined __GNUC__ */

static __inline void swap_byte (byte_t *x, byte_t *y) {
    byte_t t = *x;
    *x = *y;
    *y = t;
}

static __inline rc4_key_t prepare_key (const byte_t *__restrict key_data_ptr, const size_t key_length) {
    rc4_key_t key;
    byte_t j;
    size_t i;

    key.i = key.j = 0;

    for (i = 0; i < SEED_SIZE; i++) {
        key.state[i] = (byte_t) i;
    }

#if defined KEY_LENGTH_REMAINDER_BITWISE_AND
    if (!(key_length & (key_length - 1u))) /* If key_length is a power of 2 */
        for (i = j = 0; i < SEED_SIZE; i++) { /* Faster than modulo or branching. */
            j = (byte_t) (j + key.state[i] + key_data_ptr[i & (key_length - 1u)]) % SEED_SIZE;
            swap_byte(&key.state[i], &key.state[j]);
        }
    else
#endif 
    for (i = j = 0; i < SEED_SIZE; i++) {
        j = (byte_t) (j + key.state[i] + key_data_ptr[i % key_length]) % SEED_SIZE;
        swap_byte(&key.state[i], &key.state[j]);
    }

    return key;
}

static __inline void rc4 (byte_t *__restrict buffer_ptr, const size_t buffer_length, rc4_key_t *__restrict key) {
    size_t counter;
    byte_t i = key->i;
    byte_t j = key->j;

    for (counter = 0; counter < buffer_length; counter++) {
        i = (byte_t) (i + 1u) % SEED_SIZE;
        j = (byte_t) (key->state[i] + j) % SEED_SIZE;
        swap_byte(&key->state[i], &key->state[j]);
        buffer_ptr[counter] ^= key->state[(byte_t) (key->state[i] + key->state[j]) % SEED_SIZE];
    }

    key->i = i;
    key->j = j;
}

#endif /* RC4_H */
