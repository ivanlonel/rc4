#include "rc4.h"

static void swap_byte(byte_t *x, byte_t *y) {
    byte_t t = *x;
    *x = *y;
    *y = t;
}

void prepare_key(byte_t *key_data_ptr, size_t key_data_len, rc4_key_t *key) {
    size_t counter;
    byte_t index1 = 0;
    byte_t index2 = 0;

    key->x = 0;
    key->y = 0;

    for(counter = 0; counter < SEED_SIZE; counter++)
        key->state[counter] = (byte_t) counter;

    for(counter = 0; counter < SEED_SIZE; counter++) {
        index2 = (byte_t) ((key_data_ptr[index1] + key->state[counter] + index2) % SEED_SIZE);
        swap_byte(&key->state[counter], &key->state[index2]);
        index1 = (byte_t) ((index1 + 1u) % key_data_len);
    }
}

void rc4(byte_t *buffer_ptr, size_t buffer_len, rc4_key_t *key) {
    size_t counter;

    for(counter = 0; counter < buffer_len; counter++) {
        key->x = (key->x + 1u) % SEED_SIZE;
        key->y = (byte_t) ((key->state[key->x] + key->y) % SEED_SIZE);
        swap_byte(&key->state[key->x], &key->state[key->y]);
        buffer_ptr[counter] ^= key->state[(key->state[key->x] + key->state[key->y]) % SEED_SIZE];
    }
}
