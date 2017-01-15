#include "rc4.h"

#if !defined (__GNUC__) && !defined (__inline)
# if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define __inline inline
# else
#  define __inline
# endif
#endif

static __inline void swap_byte(byte_t *x, byte_t *y) {
    byte_t t = *x;
    *x = *y;
    *y = t;
}

rc4_key_t prepare_key(const byte_t *__restrict key_data_ptr, size_t key_data_len) {
    size_t counter;
    rc4_key_t key;

    key.x = key.y = 0; /* Initializing for temporary use inside this function. */

    for(counter = 0; counter < SEED_SIZE; counter++) {
        key.state[counter] = (byte_t) counter;
    }

    for(counter = 0; counter < SEED_SIZE; counter++) {
        key.y = (byte_t) ((key_data_ptr[key.x] + key.state[counter] + key.y) % SEED_SIZE);
        swap_byte(&key.state[counter], &key.state[key.y]);
        key.x = (byte_t) ((key.x + 1u) % key_data_len);
    }

    key.x = key.y = 0; /* Redefining for external use. */

    return key;
}

void rc4(byte_t *__restrict buffer_ptr, size_t buffer_len, rc4_key_t *__restrict key) {
    size_t counter;

    for(counter = 0; counter < buffer_len; counter++) {
        key->x = (byte_t) ((key->x + 1u) % SEED_SIZE);
        key->y = (byte_t) ((key->state[key->x] + key->y) % SEED_SIZE);
        swap_byte(&key->state[key->x], &key->state[key->y]);
        buffer_ptr[counter] ^= key->state[(key->state[key->x] + key->state[key->y]) % SEED_SIZE];
    }
}
