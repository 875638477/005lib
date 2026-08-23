#include "ask_demod.h"

void ask_demod_init(ask_demod_t *d)
{
    ask_demod_reset(d);
}

void ask_demod_reset(ask_demod_t *d)
{
    unsigned i;

    d->state = ASK_IDLE;
    d->last_level = 0U;
    d->mid_trans = 0U;
    d->bit_count = 0U;
    d->byte_acc = 0U;
    d->nbytes = 0U;
    d->packet_ready = 0U;
    for (i = 0; i < ASK_MAX_BYTES; ++i) {
        d->bytes[i] = 0U;
    }
}

uint8_t ask_checksum(const uint8_t *bytes, unsigned n)
{
    unsigned i;
    uint8_t s = 0U;

    for (i = 0; i < n; ++i) {
        s = (uint8_t)(s ^ bytes[i]);
    }
    return s;
}

int ask_packet_ok(const uint8_t *bytes, unsigned n)
{
    if (n < 2U) {
        return 0;
    }
    return ask_checksum(bytes, n - 1U) == bytes[n - 1U];
}

/*
 * 差分双相，半比特采样：
 * 偶数半周（位界）：必须有沿。
 * 奇数半周（位中）：有沿 = 0，无沿 = 1。
 */
int ask_demod_halfbit(ask_demod_t *d, uint8_t level)
{
    uint8_t edge = (uint8_t)((level ? 1U : 0U) != d->last_level);
    uint8_t bit;

    d->last_level = level ? 1U : 0U;

    if (d->mid_trans == 0U) {
        if (d->state == ASK_IDLE) {
            if (edge) {
                d->state = ASK_PREAMBLE;
                d->bit_count = 0U;
                d->nbytes = 0U;
                d->byte_acc = 0U;
                d->mid_trans = 1U;
            }
            return 0;
        }
        if (!edge) {
            ask_demod_reset(d);
            return 0;
        }
        d->mid_trans = 1U;
        return 0;
    }

    d->mid_trans = 0U;
    bit = edge ? 0U : 1U;

    if (d->state == ASK_PREAMBLE) {
        if (bit == 1U) {
            d->bit_count++;
            if (d->bit_count >= ASK_PREAMBLE_ONES) {
                d->state = ASK_DATA;
                d->bit_count = 0U;
                d->byte_acc = 0U;
            }
        } else {
            d->bit_count = 0U;
        }
        return 0;
    }

    d->byte_acc = (uint8_t)((d->byte_acc << 1) | bit);
    d->bit_count++;
    if (d->bit_count < 8U) {
        return 0;
    }
    if (d->nbytes < ASK_MAX_BYTES) {
        d->bytes[d->nbytes++] = d->byte_acc;
    }
    d->bit_count = 0U;
    d->byte_acc = 0U;
    if (d->nbytes >= 2U && ask_packet_ok(d->bytes, d->nbytes)) {
        d->packet_ready = 1U;
        d->state = ASK_IDLE;
        return 1;
    }
    return 0;
}
