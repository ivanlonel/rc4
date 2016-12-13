#ifndef RC4_H
#define RC4_H

#include <stdlib.h>
#include <stdint.h>

#define SEED_SIZE 256

typedef struct rc4_key {
    uint8_t x;
    uint8_t y;
    uint8_t state[SEED_SIZE];
} rc4_key_t;

void swap_byte(uint8_t *x, uint8_t *y);

void prepare_key(uint8_t *key_data_ptr, size_t key_data_len, rc4_key_t *key);

void rc4(uint8_t *buffer_ptr, size_t buffer_len, rc4_key_t *key);

#endif /* RC4_H */
