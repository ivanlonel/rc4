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

void prepare_key(byte_t*, size_t, rc4_key_t*);

void rc4(byte_t*, size_t, rc4_key_t*);

#endif /* RC4_H */
