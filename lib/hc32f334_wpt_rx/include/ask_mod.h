#ifndef ASK_MOD_H
#define ASK_MOD_H

#include <stdint.h>

#define ASK_MOD_PREAMBLE    11U
#define ASK_MOD_MAX_LEVELS  512U

uint8_t ask_mod_xor(const uint8_t *bytes, unsigned n);
unsigned ask_mod_encode(uint8_t *levels, unsigned max,
                        const uint8_t *payload, unsigned nbytes);

#endif
