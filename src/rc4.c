#include "rc4.h"

/* The functions declared as static inline (and defined) in the header file must be redeclared
 * for external linkage here, in case the compiler decides not to inline them. */

extern __inline void swap_byte (byte_t*, byte_t*);

extern __inline rc4_key_t prepare_key (const byte_t *__restrict, size_t);

extern __inline void rc4 (byte_t *__restrict, size_t, rc4_key_t *__restrict);
