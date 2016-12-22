#ifndef RC4_H
#define RC4_H

#include <stdlib.h>
#include <stdint.h>

#define SEED_SIZE 256

typedef uint_fast8_t byte_t;

typedef struct rc4_key {
    byte_t x;
    byte_t y;
    byte_t state[SEED_SIZE];
} rc4_key_t;

void swap_byte(byte_t *x, byte_t *y);

void prepare_key(byte_t *key_data_ptr, size_t key_data_len, rc4_key_t *key);

void rc4(byte_t *buffer_ptr, size_t buffer_len, rc4_key_t *key);

#endif /* RC4_H */
