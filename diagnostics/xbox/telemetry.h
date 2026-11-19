/* SPDX-License-Identifier: Apache-2.0 */
#ifndef KVM_DIAGNOSTIC_TELEMETRY_H
#define KVM_DIAGNOSTIC_TELEMETRY_H
#include <stdint.h>
#include <string.h>
/* Wire bytes are rendered MSB first, 32 cells/row, 18 rows, with CRC32. */
#define DIAG_BYTES 72
#define DIAG_COLS 32
#define DIAG_ROWS 18
#define DIAG_X 120
#define DIAG_Y 188
#define DIAG_CELL 12
#define DIAG_BORDER 8
#define DIAG_W 400
#define DIAG_H 232
struct diag_pad {
    uint8_t present;
    uint16_t held, last;
    uint32_t presses, releases;
};
static inline void diag_sample(struct diag_pad *p, uint16_t mask)
{
    if (mask && mask != p->held) { p->last = mask; p->presses++; }
    if (!mask && p->held) p->releases++;
    p->held = mask;
}
static inline void diag_u32(uint8_t *p, uint32_t v)
{
    for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i));
}
static inline uint32_t diag_crc(const uint8_t *p, unsigned n)
{
    uint32_t crc = 0xffffffffu;
    for (unsigned i = 0; i < n; i++) {
        crc ^= p[i];
        for (int bit = 0; bit < 8; bit++) crc = (crc >> 1) ^ ((0u - (crc & 1u)) & 0xedb88320u);
    }
    return ~crc;
}
static inline void diag_encode(uint8_t out[DIAG_BYTES], uint32_t frames,
                               uint32_t ms, uint32_t free_pages, const struct diag_pad pads[4])
{
    memcpy(out, "XKD1", 4);
    diag_u32(out + 4, frames); diag_u32(out + 8, ms); diag_u32(out + 12, free_pages);
    for (int i = 0; i < 4; i++) {
        uint8_t *p = out + 16 + i * 13;
        p[0] = pads[i].present;
        p[1] = (uint8_t)pads[i].held; p[2] = (uint8_t)(pads[i].held >> 8);
        p[3] = (uint8_t)pads[i].last; p[4] = (uint8_t)(pads[i].last >> 8);
        diag_u32(p + 5, pads[i].presses); diag_u32(p + 9, pads[i].releases);
    }
    diag_u32(out + 68, diag_crc(out, 68));
}
#endif
