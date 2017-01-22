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

rc4_key_t prepare_key (const byte_t *__restrict key_data_ptr, const size_t key_data_len) {
    rc4_key_t key;
    byte_t x, y;
    size_t counter;

    x = y = key.x = key.y = 0;

    for (counter = 0; counter < SEED_SIZE; counter++) {
        key.state[counter] = (byte_t) counter;
    }

    /* if (!(key_data_len & (key_data_len - 1))) // If key_data_len is not a power of 2
        for (counter = 0; counter < SEED_SIZE; counter++) {
            y = (byte_t) ((key_data_ptr[x] + key.state[counter] + y) % SEED_SIZE);
            x = (byte_t) ((x + 1u) & (key_data_len - 1u)); // Only works if key_data_len is a power of 2
            swap_byte(&key.state[counter], &key.state[y]);
        }
    else */ 
    for (counter = 0; counter < SEED_SIZE; counter++) {
        y = (byte_t) ((key_data_ptr[x] + key.state[counter] + y) % SEED_SIZE);
        x = (byte_t) (key_data_len - 1 == x ? 0 : x + 1); /* x = (byte_t) ((x + 1u) % key_data_len); */
        swap_byte(&key.state[counter], &key.state[y]);
    }

    return key;
}

void rc4 (byte_t *__restrict buffer_ptr, const size_t buffer_len, rc4_key_t *__restrict key) {
    size_t counter;
    byte_t x = key->x;
    byte_t y = key->y;

    for (counter = 0; counter < buffer_len; counter++) {
        x = (byte_t) ((x + 1u) % SEED_SIZE);
        y = (byte_t) ((key->state[x] + y) % SEED_SIZE);
        swap_byte(&key->state[x], &key->state[y]);
        buffer_ptr[counter] ^= key->state[(key->state[x] + key->state[y]) % SEED_SIZE];
    }
    
    key->x = x;
    key->y = y;
}
