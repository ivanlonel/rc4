#include "rc4.h"

#if !defined __inline && !defined __GNUC__ && !(defined _MSC_VER && _MSC_VER >= 1310)
# if defined __inline__
#  define __inline __inline__
# elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  define __inline inline
# else
#  define __inline
# endif
#endif

static __inline void swap_byte (byte_t *x, byte_t *y) {
    byte_t t = *x;
    *x = *y;
    *y = t;
}

rc4_key_t prepare_key (const byte_t *__restrict key_data_ptr, const size_t key_length) {
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
            j = (byte_t) ((j + key.state[i] + key_data_ptr[i & (key_length - 1u)]) % SEED_SIZE);
            swap_byte(&key.state[i], &key.state[j]);
        }
    else
#endif 
    for (i = j = 0; i < SEED_SIZE; i++) {
        j = (byte_t) ((j + key.state[i] + key_data_ptr[i % key_length]) % SEED_SIZE);
        swap_byte(&key.state[i], &key.state[j]);
    }

    return key;
}

void rc4 (byte_t *__restrict buffer_ptr, const size_t buffer_length, rc4_key_t *__restrict key) {
    size_t counter;
    byte_t i = key->i;
    byte_t j = key->j;

    for (counter = 0; counter < buffer_length; counter++) {
        i = (byte_t) ((i + 1u) % SEED_SIZE);
        j = (byte_t) ((key->state[i] + j) % SEED_SIZE);
        swap_byte(&key->state[i], &key->state[j]);
        buffer_ptr[counter] ^= key->state[(key->state[i] + key->state[j]) % SEED_SIZE];
    }
    
    key->i = i;
    key->j = j;
}
