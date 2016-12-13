#include "rc4.h"

void swap_byte(uint8_t *x, uint8_t *y) {
    uint8_t *t = x;
    x = y;
    y = t;
}

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len, rc4_key *key) {
    size_t counter;
    uint8_t index1 = 0;
    uint8_t index2 = 0;

    key->x = 0;
    key->y = 0;

    for(counter = 0; counter < SEED_SIZE; counter++)
        key->state[counter] = counter;

    for(counter = 0; counter < SEED_SIZE; counter++) {
        index2 = (key_data_ptr[index1] + key->state[counter] + index2) % SEED_SIZE;
        swap_byte(&key->state[counter], &key->state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void rc4(uint8_t *buffer_ptr, size_t buffer_len, rc4_key *key) {
    size_t counter;

    for(counter = 0; counter < buffer_len; counter++) {
        key->x = (key->x + 1) % SEED_SIZE;
        key->y = (key->state[key->x] + key->y) % SEED_SIZE;
        swap_byte(&key->state[key->x], &key->state[key->y]);
        buffer_ptr[counter] ^= key->state[(key->state[key->x] + key->state[key->y]) % SEED_SIZE];
    }
}
