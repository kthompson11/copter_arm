#include "cobs.h"

int cobs_stuff(const uint8_t *unstuffed, uint8_t *stuffed, int len)
{
    int offset_dest = 0;
    int offset_value = 1;
    int dest = 1;
    int src = 0;

    while (src != len) {
        if (offset_value == 255) {
            stuffed[offset_dest] = offset_value;
            offset_dest = dest;
            dest += 1;
            offset_value = 1;
        } else if ((unstuffed[src] == 0)) {
            stuffed[offset_dest] = offset_value;
            offset_dest = dest;
            offset_value = 1;
            dest += 1;
            src += 1;
        } else {
            stuffed[dest] = unstuffed[src];
            offset_value += 1;
            dest += 1;
            src += 1;
        }
    }

    /* add delimiter and final offset */
    stuffed[offset_dest] = offset_value;
    stuffed[dest] = 0;
    dest += 1;

    return dest;
}