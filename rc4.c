#include "rc4.h"

void swap_byte(uint8_t *x, uint8_t *y) {
    uint8_t t = *x;
    *x = *y;
    *y = t;
}

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len, rc4_key *key) {
    uint8_t *state;
    uint8_t index1 = 0;
    uint8_t index2 = 0;
    size_t counter;

    key->x = 0;
    key->y = 0;

    state = &key->state[0];
    for(counter = 0; counter < SEED_SIZE; counter++)
        state[counter] = counter;

    for(counter = 0; counter < SEED_SIZE; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % SEED_SIZE;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void rc4(uint8_t *buffer_ptr, size_t buffer_len, rc4_key *key) {
    uint8_t x;
    uint8_t y;
    uint8_t *state;
    uint8_t xorIndex;
    size_t counter;

    x = key->x;
    y = key->y;
    state = &key->state[0];
    for(counter = 0; counter < buffer_len; counter++) {
        x = (x + 1) % SEED_SIZE;
        y = (state[x] + y) % SEED_SIZE;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % SEED_SIZE;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}
