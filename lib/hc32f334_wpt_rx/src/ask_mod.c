#include "ask_mod.h"

uint8_t ask_mod_xor(const uint8_t *bytes, unsigned n)
{
    unsigned i;
    uint8_t s = 0U;

    for (i = 0; i < n; ++i) {
        s = (uint8_t)(s ^ bytes[i]);
    }
    return s;
}

static int push_bit(uint8_t *levels, unsigned *n, unsigned max, uint8_t *level, uint8_t bit)
{
    if (*n + 2U > max) {
        return -1;
    }
    *level ^= 1U;
    levels[(*n)++] = *level;
    if (bit == 0U) {
        *level ^= 1U;
    }
    levels[(*n)++] = *level;
    return 0;
}

unsigned ask_mod_encode(uint8_t *levels, unsigned max,
                        const uint8_t *payload, unsigned nbytes)
{
    uint8_t level = 0U;
    unsigned n = 0U;
    unsigned i;
    int b;
    uint8_t pkt[32];
    unsigned pn;

    if (nbytes == 0U || nbytes + 1U > sizeof(pkt)) {
        return 0U;
    }
    for (i = 0; i < nbytes; ++i) {
        pkt[i] = payload[i];
    }
    pkt[nbytes] = ask_mod_xor(payload, nbytes);
    pn = nbytes + 1U;

    for (i = 0; i < ASK_MOD_PREAMBLE; ++i) {
        if (push_bit(levels, &n, max, &level, 1U) != 0) {
            return 0U;
        }
    }
    for (i = 0; i < pn; ++i) {
        for (b = 7; b >= 0; --b) {
            uint8_t bit = (uint8_t)((pkt[i] >> b) & 1U);
            if (push_bit(levels, &n, max, &level, bit) != 0) {
                return 0U;
            }
        }
    }
    return n;
}
